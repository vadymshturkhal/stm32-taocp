#pragma once
#include <cstdint>
#include <cstddef>
#include "wait_list.hpp"
#include "storage_pool.hpp"
#include "elevator_node.hpp"

class ElevatorSimulation;

class Users {
public:
    explicit Users(ElevatorSimulation& s) noexcept : simulation(s), USER1(s, &U1) {}

    void start(std::uint32_t quantity) noexcept;
    static void U5(WaitNode& node, std::uint32_t now) noexcept;
    static void U6(WaitNode& node, std::uint32_t now) noexcept;

private:
    ElevatorSimulation& simulation;
    ElevatorNode USER1;
    std::uint32_t users_quantity = 0;
    std::uint32_t user_id = 0;

    // Users in the system at once (not in total); the most seen is 7-10, depending on VALUES
    static constexpr std::size_t POOL_SIZE = 16;
    StoragePool<ElevatorNode> pool{POOL_SIZE};
    bool parked = false;    // U1 waits for a free node (park and wake)

    static void U1(WaitNode& node, std::uint32_t now) noexcept;
    static void U2(WaitNode& user, std::uint32_t now) noexcept;
    static void U3(WaitNode& user, std::uint32_t now) noexcept;
    static void U4(WaitNode& user, std::uint32_t now) noexcept;
    void U4A(ElevatorNode& user) noexcept;
    void U6_impl(ElevatorNode& user, bool is_print) noexcept;
};
