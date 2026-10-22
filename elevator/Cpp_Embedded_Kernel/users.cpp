#include <cassert>
#include <cstdint>
#include "users.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"

void Users::start(std::uint32_t quantity) noexcept {
    // USER1 node represents action U1 and it is initially the sole entry in the WAIT list
    users_quantity = quantity; 
    simulation.immed(USER1);
}

void Users::U1(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U2(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U3(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U4(WaitNode& node, std::uint32_t now) noexcept {}
void Users::U4A(ElevatorNode& user) noexcept {}

void Users::U5(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    ElevatorNode& user = static_cast<ElevatorNode&>(node);

    trace(sim, "U5", "User %u gets in", user.id);

    // 1. This user now leaves QUEUE and enters ELEVATOR

    // Delete User from QUEUE[IN]
    ElevatorList::delete_node(user);

    // Insert it at right of ELEVATOR
    sim.ELEVATOR_LIST.insert_at_rear(user);

    // 2. Set CALLCAR[OUT] = 1
    sim.CALLS[user.OUT] |= CALLCAR;

    Elevator& elevator = sim.elevator;
    // 3. Jump if STATE != NEUTRAL
    if (elevator.STATE != 0) {
        // J5NZ CYCLE
        return;
    }

    // Knuth's style: set STATE = GOINGUP or GOINGDOWN as appropriate
    elevator.STATE = static_cast<std::int32_t>(user.OUT) - static_cast<std::int32_t>(elevator.FLOOR);

    // 4. Remove action E5 from WAIT list
    sim.cancel(elevator.ELEV2);

    // Restart E5 after 25 units
    std::uint32_t delay = 25;
    elevator.E5A(delay);
}

// [Get out]
void Users::U6(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Users& users = sim.users;
    users.U6_impl(static_cast<ElevatorNode&>(node), true);
}

void Users::U6_impl(ElevatorNode& user, bool is_print) noexcept {
    if (is_print) {
        trace(simulation, "U6", "User %u gets out, leaves the system", user.id);
    }

    // JMP DELETE: leave the QUEUE or the ELEVATOR
    ElevatorList::delete_node(user);

    // Not in MIX -- Python relies on garbage collection here; 
    // C++ manages the storage pool manually, so the node must be returned once the user leaves
    assert(!user.is_held() && !user.is_in_queue());
    pool.push(user);

    // Park and wake: U1 was waiting for a free node
    if (parked) {
        parked = false;
        simulation.immed(USER1);
    }
}