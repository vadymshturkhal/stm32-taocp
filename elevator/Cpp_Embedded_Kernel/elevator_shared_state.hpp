#pragma once
#include <cstdint>

#include "kernel.hpp"
#include "elevator_list.hpp"
#include "elevator.hpp"
#include "users.hpp"
#include "elevator_constants.hpp"

// SharedState in C version
class ElevatorState {
public:
    explicit ElevatorState(Kernel& k) noexcept : kernel(k) {}

    Kernel& kernel;
    std::uint32_t TIME = 0;
    std::uint32_t CALLS[FLOORS]{};
    ElevatorList ELEVATOR_LIST;
    ElevatorList QUEUE[FLOORS];
    Elevator elevator{*this};
    Users users{*this};
};
