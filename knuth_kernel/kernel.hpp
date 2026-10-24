#pragma once
#include <cstddef>
#include <optional>
#include "wait_list.hpp"

class Kernel {
public:
    // immed for newly started program
    void start(WaitNode& node, std::uint32_t now) noexcept {
        list_.immed(node, now);
    }

    void hold(WaitNode& node, std::uint32_t now) noexcept {
        list_.hold(node, now);
    }

    void hold(WaitNode& node, std::uint32_t now, std::uint32_t delay) noexcept {
        list_.hold(node, now, delay);
    }

    void holdc(WaitNode& node, std::uint32_t now, std::uint32_t delay, WaitNode::NextInst next_inst) noexcept {
        list_.holdc(node, now, delay, next_inst);
    }

    // A literal 0 or nullptr as the step would hold the node with no step (see WaitList)
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, int) = delete;
    void holdc(WaitNode&, std::uint32_t, std::uint32_t, std::nullptr_t) = delete;

    void cancel(WaitNode& node) noexcept {
        list_.delete_node(node);
    }

    // kernel's heartbeat
    bool step(std::uint32_t now) noexcept {
        if(!list_.is_node_ready(now)) return false;

        // can remove if guard
        if (WaitNode* n = list_.delete_first()) n->run(now);
        return true;
    }

    std::optional<std::uint32_t> get_next_time() const noexcept {
        return list_.get_next_time();
    }

    // Length and sortin walk counters: all zero unless KNUTH_KERNEL_STATS is defined
    [[nodiscard]] const WaitListStats& list_stats() const noexcept { return list_.stats(); }

private:
    WaitList list_;
};
