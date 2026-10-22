#include <cassert>
#include <cstdint>
#include "users.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"

void Users::start(std::uint32_t quantity) noexcept {
    users_quantity = quantity; 
    simulation.immed(USER1);
}
void Users::U1(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U2(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U3(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U4(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U4A(ElevatorNode& user) noexcept {}
void Users::U5(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U6(WaitNode& node, std::uint32_t now) noexcept {}

void Users::U6_impl(ElevatorNode& user, bool is_print) noexcept {
    if (is_print) {
        trace(simulation, "U6", "User %u gets out, leaves the system", user.id);
    }

    // JMP DELETE: leave the QUEUE or the ELEVATOR
    ElevatorList::delete_node(user);

	// Not in MIX -- Python relies on garbage collection here; 
    // C++ manages the storage pool manually, so the node must be returned once the user leaves
    pool.push(user);

    // Park and wake: U1 was waiting for a free node
    if (parked) {
        parked = false;
        simulation.immed(USER1);
    }
}
