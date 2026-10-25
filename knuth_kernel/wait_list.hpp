#pragma once
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>

// Knuth (§2.2.5): about 10 nodes wait at a time, and SORTIN is the main cost.
// WaitListStats checks both. The counters run only with KNUTH_KERNEL_STATS
// defined; set it for the whole program (a compiler flag), so every file sees the
// same WaitList.
#ifdef KNUTH_KERNEL_STATS
inline constexpr bool WAIT_LIST_STATS = true;
#else
inline constexpr bool WAIT_LIST_STATS = false;
#endif

struct WaitListStats {
    std::uint32_t sortins = 0;      // insertions by time (hold, holdc)
    std::uint32_t length_max = 0;   // nodes already waiting when one is sorted in
    std::uint32_t length_sum = 0;   // / sortins = the average length
    std::uint32_t walk_max = 0;     // nodes one sortin passes on its way from the tail
    std::uint32_t walk_sum = 0;     // / sortins = the average walk
};

// forward declaration
class WaitList;

class WaitNode {
public:
    using NextInst = void (*)(WaitNode&, std::uint32_t now) noexcept;

    void run(std::uint32_t now) noexcept {
        // hold, holdc and immed assert a step; the test stays so that a release
        // build never jumps to address 0 if a step is cleared while a node waits
        assert(NEXTINST != nullptr && "run: a node with no step");
        if(NEXTINST) NEXTINST(*this, now);
    }

    WaitNode() = default;
    constexpr WaitNode(NextInst next_inst, std::uint32_t delay) noexcept 
        : NEXTINST(next_inst), DELAY(delay) {}

    [[nodiscard]] bool is_held() const noexcept { return right != this; }
    [[nodiscard]] std::uint32_t get_next_time() const noexcept { return NEXTTIME; }
    [[nodiscard]] NextInst get_next_inst() const noexcept { return NEXTINST; }
    void set_next_inst(NextInst next_inst) noexcept { NEXTINST = next_inst; }

    WaitNode(const WaitNode&) = delete;
    WaitNode& operator=(const WaitNode&) = delete;

private:
    friend class WaitList;
    template <std::derived_from<WaitNode> T> friend class StoragePool;   // uses left for the AVAIL list

    // On a pool's AVAIL list: in no list (right points to itself), but left links
    // the next free node. A node in use points to itself both ways when it is in
    // no list, so only a free node looks like this. No list operation may touch it:
    // unlinking it would write into the next free node and cut AVAIL.
    [[nodiscard]] bool is_on_avail() const noexcept { return right == this && left != this; }

    WaitNode* left = this;
    WaitNode* right = this;
    std::uint32_t NEXTTIME = 0;
    NextInst NEXTINST = nullptr;
    std::uint32_t DELAY = 0;
};

class WaitList {
public:
    using NextInst = WaitNode::NextInst;

    static constexpr bool is_less(std::uint32_t a, std::uint32_t b) noexcept {
        return static_cast<std::int32_t>(a - b) < 0;
    }

    void hold(WaitNode& node, std::uint32_t now, std::uint32_t delay) noexcept {
        assert(!node.is_on_avail() && "hold: the node is free (on a pool's AVAIL)");
        assert(node.NEXTINST != nullptr && "hold: a node with no step would never run");
        if (node.is_on_avail()) { ++refused_; return; }   // release builds: refuse, and count it

        // Schedule node to run delay units from now
        node.NEXTTIME = now + delay;
        
        unlink_node(node);
        sortin(node);
    }

    // Use node's DELAY
    void hold(WaitNode& node, std::uint32_t now) noexcept {
        assert(!node.is_on_avail() && "hold: the node is free (on a pool's AVAIL)");
        assert(node.NEXTINST != nullptr && "hold: a node with no step would never run");
        if (node.is_on_avail()) { ++refused_; return; }   // release builds: refuse, and count it

        // Schedule node to run delay units from now
        node.NEXTTIME = now + node.DELAY;
        
        unlink_node(node);
        sortin(node);
    }

    void holdc(WaitNode& node, std::uint32_t now, std::uint32_t delay, NextInst next_inst) noexcept {
        // Set NEXTINST and then invoke hold
        node.NEXTINST = next_inst;
        hold(node, now, delay);
    }

    // A literal 0 or nullptr as the step converts to a null NextInst: the node would
    // be held with no step, and the program would stop without a sound (it is also
    // the easy slip of writing holdc(node, now, 0) for "hold 0 with the same step").
    // These make both a compile error
    void holdc(WaitNode&, std::uint32_t, int) = delete;
    void holdc(WaitNode&, std::uint32_t, std::nullptr_t) = delete;
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, int) = delete;
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, std::nullptr_t) = delete;

    // Use node's DELAY
    void holdc(WaitNode& node, std::uint32_t now, NextInst next_inst) noexcept {
        // Set NEXTINST and then invoke hold
        node.NEXTINST = next_inst;
        hold(node, now);
    }

    // Insert node first in WAIT list
    void immed(WaitNode& node, std::uint32_t now) noexcept {
        assert(!node.is_on_avail() && "immed: the node is free (on a pool's AVAIL)");
        assert(node.NEXTINST != nullptr && "immed: a node with no step would never run");
        if (node.is_on_avail()) { ++refused_; return; }   // release builds: refuse, and count it

        // Unlink node
        unlink_node(node);

        // Set NEXTTIME
        node.NEXTTIME = now;

        // Insert
        node.left = &head;
        node.right = head.right;
        head.right->left = &node;
        head.right = &node;

        if constexpr (WAIT_LIST_STATS) ++length_;
    }

    [[nodiscard]] bool is_node_ready(std::uint32_t now) const noexcept {
        return !is_empty() && !is_less(now, head.right->NEXTTIME);
    }

    [[nodiscard]] WaitNode* delete_first() noexcept {
        if (is_empty()) return nullptr;

        WaitNode& n = *head.right;
        delete_node(n);

        return &n;
    }

    // Cancel. A node in no list (never held, already cancelled, or running now)
    // stays as it is; a free node (on a pool's AVAIL) is refused
    void delete_node(WaitNode& node) noexcept {
        assert(!node.is_on_avail() && "cancel: the node is free (on a pool's AVAIL)");
        if (node.is_on_avail()) { ++refused_; return; }   // release builds: refuse, and count it

        // Unlink node
        unlink_node(node);

        // Point to itself
        node.left = &node;
        node.right = &node;
    }

    [[nodiscard]] bool is_empty() const noexcept {
        return head.right == &head;
    }

    [[nodiscard]] std::optional<std::uint32_t> get_next_time() const noexcept {
        if(is_empty()) return std::nullopt;
        return head.right->NEXTTIME;
    }

    // Operations refused because the node was free (on a pool's AVAIL). Release
    // builds only (debug builds assert first): a refused hold leaves the node
    // never running, so this is the one place it shows.
    [[nodiscard]] std::uint32_t refused() const noexcept { return refused_; }

    // All zero unless KNUTH_KERNEL_STATS is defined
    [[nodiscard]] const WaitListStats& stats() const noexcept { return stats_; }
    [[nodiscard]] std::uint32_t length() const noexcept { return length_; }
         
private:
    WaitNode head;
    std::uint32_t refused_ = 0;     // see refused()
    std::uint32_t length_ = 0;      // nodes in the list (counted with KNUTH_KERNEL_STATS)
    WaitListStats stats_{};

    void unlink_node(WaitNode& node) noexcept {
        // A node that is not held points to itself: unlinking it changes nothing
        if constexpr (WAIT_LIST_STATS) {
            if (node.right != &node) --length_;
        }
        node.left->right = node.right;
        node.right->left = node.left;
    }

    // Sort node into WAIT list using left and right
    void sortin(WaitNode& C) noexcept {
        // Sentinel: the head gets C's own time. is_less(C, head) is then false, so
        // the walk always stops at the head and the loop needs no "P != &head"
        // test. Knuth's fixed sentinel (a head time of 0) cannot work on a clock
        // that wraps: past 2^31 every time counts as "before" 0, and the walk would
        // go round the circle for ever. One store per hold instead of one compare
        // and branch per node passed
        head.NEXTTIME = C.NEXTTIME;

        WaitNode* P = head.left;  // Last node
        std::uint32_t walked = 0;

        // Compare NEXTTIME fields right to left
        while (is_less(C.NEXTTIME, P->NEXTTIME)) {
            P = P->left;
            if constexpr (WAIT_LIST_STATS) ++walked;
        }

        if constexpr (WAIT_LIST_STATS) {
            ++stats_.sortins;
            stats_.length_sum += length_;
            if (length_ > stats_.length_max) stats_.length_max = length_;
            stats_.walk_sum += walked;
            if (walked > stats_.walk_max) stats_.walk_max = walked;
            ++length_;
        }

        // Insert node to WAIT list
        WaitNode* Q = P->right;
        C.right = Q;
        C.left = P;
        P->right = &C;
        Q->left = &C;
    }
};
