#pragma once
#include <cstdint>
#include "wait_list.hpp"

class ElevatorState;

class ElevatorNode : public WaitNode {
public:
    ElevatorNode() noexcept = default;
    ElevatorNode(ElevatorState& s, NextInst next_inst) noexcept :
        WaitNode(next_inst, 0), state(&s) {}

    ElevatorState* state = nullptr;

    ElevatorNode* left2 = this;
    ElevatorNode* right2 = this;

    // User related fields
    std::uint32_t IN = 0;
    std::uint32_t OUT = 0;
    std::uint32_t GIVEUPTIME = 0;

    std::uint32_t id = 0;  // For trace printing
};
