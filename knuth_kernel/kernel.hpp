#pragma once
#include <cstdint>
#include <cstddef>
#include <optional>
#include <array>
#include <cassert>
#include "wait_list.hpp"

/*
 * The levels' priorities: the port's kernel_levels.hpp when the build has it on its
 * include path (the board builds), else 14 and 15 (the PC builds: the tests and the
 * PC elevator). <>, not "": it must not find a file next to this one. A file that
 * sees both would define kernel_priorities twice, and the build stops.
 */
#if __has_include(<kernel_levels.hpp>)
#include <kernel_levels.hpp>
#else
inline constexpr std::array<std::uint32_t, 2> kernel_priorities{14, 15};
#endif

class Kernel {
public:
    constexpr Kernel() noexcept = default;

    Kernel(const Kernel&) = delete;
    Kernel& operator=(const Kernel&) = delete;

    void start(WaitNode& node, std::uint32_t now) noexcept 
    {
        list_of(node)->immed(node, now);
    }

    void hold(WaitNode& node, std::uint32_t now) noexcept 
    {
        list_of(node)->hold(node, now);
    }

    void hold(WaitNode& node, std::uint32_t now, std::uint32_t delay) noexcept 
    {
        list_of(node)->hold(node, now, delay);
    }

    void holdc(WaitNode& node, std::uint32_t now, std::uint32_t delay, WaitNode::NextInst next_inst) noexcept 
    {
        list_of(node)->holdc(node, now, delay, next_inst);
    }

    // A literal 0 or nullptr as the step would hold the node with no step (see WaitList)
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, int) = delete;
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, std::nullptr_t) = delete;

    void cancel(WaitNode& node) noexcept 
    {
        list_of(node)->delete_node(node);
    }

    template <std::uint32_t P>
    bool step(std::uint32_t now) noexcept 
    {
        /* i is computed at compile time: P = 14 → 0, P = 15 → 1 */
        constexpr std::size_t i = index_of(P);
        static_assert(i < priorities.size(), "step: P isn't a kernel level");

        /* step<14> compiled to = *(kernel + offset of wait_lists[i])
         * Example: 
         * ldr  r0, [r6, #84]  ; head.right
         * cmp  r0, r8         ; == &head? 
         * Two instructions instead of four due to using template and constexpr.
         * 
         * Measured, variant A (a table of WaitList* by priority, step(now, 14)):
         * ldr  ip, [r6, #240]  ; priority_levels[14]
         * cmp  ip, #0 / beq    ; null?
         * ldr  r0, [ip, #4]    ; head.right
         * cmp  r0, ip          ; == &head?
         */
        WaitList& list = wait_lists[i];

        if(!list.is_node_ready(now)) return false;

        // can remove if guard
        if (WaitNode* n = list.delete_first()) {
            if (n->get_next_inst() == nullptr) ++null_steps_[i];
            n->run(now);
        }
        return true;
    }

    /* Level P's earliest NEXTTIME, or empty if its list is empty */
    template <std::uint32_t P>
    std::optional<std::uint32_t> get_next_time() const noexcept
    {
        constexpr std::size_t i = index_of(P);
        static_assert(i < priorities.size(), "get_next_time: P isn't a kernel level");
        return wait_lists[i].get_next_time();
    }

    /* Level P's length and sortin walk counters: all zero unless KNUTH_KERNEL_STATS is defined */
    template <std::uint32_t P>
    [[nodiscard]] const WaitListStats& list_stats() const noexcept
    {
        constexpr std::size_t i = index_of(P);
        static_assert(i < priorities.size(), "list_stats: P isn't a kernel level");
        return wait_lists[i].stats();
    }

    /* 
     * Puts node on priority level P.
     * All later operations with the node will use only attached level.
     */
    template <std::uint32_t P>
    void attach(WaitNode& node) noexcept
    {
        constexpr std::size_t i = index_of(P);
        static_assert(i < priorities.size(), "attach: P isn't a kernel level");
        assert(!node.is_held() && "attach: the node waits in a list");
        node.list_ = &wait_lists[i];
    }
    
    /* Level P's refusals: its steps with no NEXTINST and its list's refused hold/immed */
    template <std::uint32_t P>
    [[nodiscard]] std::uint32_t refused() const noexcept
    {
        constexpr std::size_t i = index_of(P);
        static_assert(i < priorities.size(), "refused: P isn't a kernel level");
        return null_steps_[i] + wait_lists[i].refused();
    }

    /* All levels' refusals */
    [[nodiscard]] std::uint32_t refused() const noexcept
    {
        std::uint32_t sum = 0;
        for (const std::uint32_t n : null_steps_) sum += n;               // steps with no NEXTINST, both levels
        for (const WaitList& list : wait_lists) sum += list.refused();    // each list's refused hold/immed (a node on AVAIL)
        return sum;
    }

private:
    static constexpr auto priorities = kernel_priorities;

    /* Array with all available wait lists */
    std::array<WaitList, priorities.size()> wait_lists{};

    /* For increasing null_steps_ in step */
    std::array<std::uint32_t, priorities.size()> null_steps_{};

    /* Compile time calculation */
    static constexpr std::size_t index_of(std::uint32_t priority) noexcept
    {
        for (std::size_t i = 0; i < priorities.size(); ++i)
        {
            if (priorities[i] == priority) return i;
        }
        
        return priorities.size();
    }

    /* The node's WAIT list: one load, no checks in release */
    static WaitList* list_of(const WaitNode& node) noexcept
    {
        assert(node.get_list() && "the node isn't attached to a level: kernel.attach<P>(node)");
        return node.get_list();
    }
};
