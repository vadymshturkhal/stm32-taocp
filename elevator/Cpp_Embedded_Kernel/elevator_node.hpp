#pragma once
#include <cstdint>
#include "wait_list.hpp"

class ElevatorSimulation;

class ElevatorNode : public WaitNode {
public:
    ElevatorNode() noexcept = default;
    ElevatorNode(ElevatorSimulation& s, NextInst next_inst) noexcept :
        WaitNode(next_inst, 0), simulation(&s) {}

    // Same trick Linux uses: list_del_init() + list_empty(&node), 
    // asks "is this node in any list?"
    [[nodiscard]] bool is_in_queue() const noexcept {return right2 != this; }

    // Back pointer state field, replaced global state
    ElevatorSimulation* simulation = nullptr;

    ElevatorNode* left2 = this;
    ElevatorNode* right2 = this;

    // User related fields
    std::uint32_t IN = 0;
    std::uint32_t OUT = 0;
    std::uint32_t GIVEUPTIME = 0;

    std::uint32_t id = 0;  // For trace printing
};
