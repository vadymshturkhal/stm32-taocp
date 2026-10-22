#include <cassert>
#include <cstdint>
#include "users.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"
#include "elevator.hpp"
#include "values.hpp" 

void Users::start(std::uint32_t quantity) noexcept {
    // USER1 node represents action U1 and it is initially the sole entry in the WAIT list
    users_quantity = quantity; 
    simulation.immed(USER1);
}

void Users::U1(WaitNode& node, std::uint32_t now) noexcept {}

// [Signal and wait]
void Users::U2(ElevatorNode& user) noexcept {
    Elevator& elevator = simulation.elevator;

    // 1. If FLOOR == IN
    if (elevator.FLOOR == user.IN) {
        // 2. And if the elevator's next action is step E6
        // (if the elevator doors are now closing)
        if (elevator.ELEV1.get_next_inst() == &Elevator::E6) {
            // Reposition it at E3
            elevator.ELEV1.set_next_inst(&Elevator::E3);

            // No DELETEW, immed unlinks ELEV1 first

            // JMP IMMED
            simulation.immed(elevator.ELEV1);

            // JMP U3
            U3(user);
            return;
        }

        // 3. If D3 != 0
        if (elevator.D3 != 0) {
            // Set D3 = 0, D1 to a nonzero value and start up the elevator's activity E4 again
            elevator.D1 = 1;
            elevator.D3 = 0;

            // JMP IMMED
            simulation.immed(elevator.ELEV1);

            // JMP U3
            U3(user);
            return;
        }
    }

    // 4. In all other cases the user sets CALLUP[IN] = 1 or CALLDOWN[IN] = 1
    // according as OUT > IN or OUT < IN
    if (user.OUT > user.IN) {
        simulation.CALLS[user.IN] |= CALLUP;
    } else {
        simulation.CALLS[user.IN] |= CALLDOWN;
    }

    // 5. And if D2 == 0 or the elevator in its "dormant" position E1, DECISION is performed
    if (elevator.D2 == 0 || elevator.ELEV1.get_next_inst() == &Elevator::E1) {
        // U2 runs from U1 step and decision only checks caller == E6
        simulation.decision(&U1);
    }

    U3(user);
}

// [Enter queue]
void Users::U3(ElevatorNode& user) noexcept {
    // Insert node at right end of QUEUE[IN]
    simulation.QUEUE[user.IN].insert_at_rear(user);
    U4A(user);
}

// [Give up]
void Users::U4(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;
    Users& users = sim.users;
    ElevatorNode& user = static_cast<ElevatorNode&>(node);

    // If the user's IN floor differs from the elevator's current FLOOR: give up
    if (user.IN != elevator.FLOOR) {
        trace(sim, "U4", "User %u decides to give up, leaves the system", user.id);
        users.U6_impl(user, false);
        return;
    }

    // JANZ U4A: doors still busy: reschedule U4
    if (elevator.D1 != 0) {
        users.U4A(user);
        return;
    }

    trace(sim, "U4", "User %u decides to give up, leaves the system", user.id);
    users.U6_impl(user, false);
}

// [Wait GIVEUPTIME units]
void Users::U4A(ElevatorNode& user) noexcept {
    simulation.holdc(user, user.GIVEUPTIME, &U4);
}

// [Get in]
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