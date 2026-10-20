#pragma once
#include <cstdint>
#include "wait_list.hpp"

class ElevatorSimulation;

class Users {
public:
    explicit Users(ElevatorSimulation& s) noexcept : simulation(s) {}

    static void U5(WaitNode& node, std::uint32_t now) noexcept;
    static void U6(WaitNode& node, std::uint32_t now) noexcept;

private:
    ElevatorSimulation& simulation;
};
