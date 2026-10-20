#pragma once
#include <cstdint>
#include "elevator_constants.hpp"
#include "elevator_node.hpp"

class ElevatorSimulation;

class Elevator {
public:
    explicit Elevator(ElevatorSimulation& s) noexcept
        : ELEV1(s, &E1), ELEV2(s, &E5), ELEV3(s, &E9), simulation(s) {}

    std::int32_t STATE = 0; // neutral state
    std::uint32_t FLOOR = HOME_FLOOR;
    std::uint32_t D1 = 0;  // a variable that is zero except during the time people are getting in or out of the elevator
    std::uint32_t D2 = 0;  // a variable that becomes zero if the elevator has sat on one floor without moving for 30 sec or more
    std::uint32_t D3 = 0;  // a variable that is zero except when the doors are open but nobody is getting in or out of the elevator

    ElevatorNode ELEV1;  // This node represents the elevator actions, except for E5 and E9
    ElevatorNode ELEV2;  // Represents the independent elevator action at E5
    ElevatorNode ELEV3;  // Represents the independent elevator action at E9

    void E1A() noexcept;
    static void E1(WaitNode& node, std::uint32_t now) noexcept;
    static void E3(WaitNode& node, std::uint32_t now) noexcept;
    static void E5(WaitNode& node, std::uint32_t now) noexcept;
    static void E6(WaitNode& node, std::uint32_t now) noexcept;
    static void E9(WaitNode& node, std::uint32_t now) noexcept;
    void E5A(std::uint32_t delay) noexcept;  // Run E5 after some delay

private:
    ElevatorSimulation& simulation;
    bool first_search = false;  // Look at E3-E4

    static void E2(WaitNode& node, std::uint32_t now) noexcept;
    static void E4(WaitNode& node, std::uint32_t now) noexcept;
    static void E7(WaitNode& node, std::uint32_t now) noexcept;
    static void E7_continue(WaitNode& node, std::uint32_t now) noexcept;
    static void E8(WaitNode& node, std::uint32_t now) noexcept;
    static void E8_continue(WaitNode& node, std::uint32_t now) noexcept;

    void E2A(std::uint32_t delay) noexcept;  // Run E2 after delay
    void E4A(std::uint32_t delay) noexcept;
    void E6B(Elevator& elevator) noexcept;
};
