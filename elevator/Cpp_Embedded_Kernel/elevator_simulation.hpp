#pragma once
#include <cstdint>
#include <cassert>

#include "kernel.hpp"
#include "elevator_list.hpp"
#include "elevator.hpp"
#include "users.hpp"
#include "elevator_constants.hpp"

// SharedState in C version
class ElevatorSimulation {
public:
    explicit ElevatorSimulation(Kernel& k) noexcept : kernel(k) {}

    Kernel& kernel;
    std::uint32_t t0 = 0;  // kernel tick at which the elevator started
    std::uint32_t TIME = 0;
    std::uint32_t CALLS[FLOORS]{};
    ElevatorList ELEVATOR_LIST;
    ElevatorList QUEUE[FLOORS];
    Elevator elevator{*this};
    Users users{*this};

    // The wrappers for deterministic TIME: units in -> kernel ticks out
    std::uint32_t ticks() const noexcept { 
        return t0 + TIME * TICKS_PER_TIME_UNIT; 
    }
    
    void hold(WaitNode& n, std::uint32_t delay) noexcept {
        kernel.hold(n, ticks(), delay * TICKS_PER_TIME_UNIT); 
    }

    void holdc(WaitNode& n, std::uint32_t delay, WaitNode::NextInst next_inst) noexcept {
        kernel.holdc(n, ticks(), delay * TICKS_PER_TIME_UNIT, next_inst);
    }

    void immed(WaitNode& n) noexcept { kernel.start(n, ticks()); }
    void cancel(WaitNode& n) noexcept { kernel.cancel(n); }
    void decision(WaitNode::NextInst caller) noexcept;
};

inline ElevatorSimulation& begin_step(WaitNode& n) noexcept {
    auto& node = static_cast<ElevatorNode&>(n);
    
    assert(node.state != nullptr);
    
    node.state->TIME = (node.next_time() - node.state->t0) / TICKS_PER_TIME_UNIT;
    return *node.state;
}
