#pragma once
#include <cstdint>
#include <cstddef>
#include "wait_list.hpp"
#include "storage_pool.hpp"
#include "elevator_node.hpp"
#include "values.hpp"
#include "elevator_constants.hpp"

class ElevatorSimulation;

class Users {
public:
    explicit Users(ElevatorSimulation& s) noexcept : simulation(s), USER1(s, &U1) {}

    void start(std::uint32_t quantity) noexcept;
    [[nodiscard]] bool done() const noexcept;   // every user created and gone
    static void U5(WaitNode& node, std::uint32_t now) noexcept;
    static void U6(WaitNode& node, std::uint32_t now) noexcept;

private:
    ElevatorSimulation& simulation;
    ElevatorNode USER1;
    Values values{VALUES_SEED};  // from constants
    std::uint32_t users_quantity = 0;
    std::uint32_t user_id = 0;
    std::uint32_t active_users = 0;   // in the system now: add_user +1, U6_impl -1

    // Users in the system at once (not in total); the most seen is 7-10, depending on VALUES
    static constexpr std::size_t POOL_SIZE = 16;
    StoragePool<ElevatorNode> pool{POOL_SIZE};
    bool parked = false;    // U1 waits for a free node (park and wake)

    void add_user(ElevatorNode& user, std::uint32_t in, std::uint32_t out, std::uint32_t giveuptime) noexcept;
    static void U1(WaitNode& node, std::uint32_t now) noexcept;
    void U2(ElevatorNode& user) noexcept;
    void U3(ElevatorNode& user) noexcept;
    static void U4(WaitNode& user, std::uint32_t now) noexcept;
    void U4A(ElevatorNode& user) noexcept;
    void U6_impl(ElevatorNode& user, bool is_print) noexcept;
};
