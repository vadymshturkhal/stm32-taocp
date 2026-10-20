#include <cassert>
#include <cstdint>
#include "wait_list.hpp"
#include "elevator.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"
#include "users.hpp"

void Elevator::E1A() noexcept { ELEV1.set_next_inst(&E1); }
void Elevator::E2A(std::uint32_t delay) noexcept { simulation.holdc(ELEV1, delay, &E2); }
void Elevator::E4A(std::uint32_t delay) noexcept { simulation.holdc(ELEV1, delay, &E4); }
void Elevator::E5A(std::uint32_t delay) noexcept { simulation.holdc(ELEV2, delay, &E5); }

// [Wait for call]
void Elevator::E1(WaitNode& /*node*/, std::uint32_t /*now*/) noexcept {}

// [Change of state?]
void Elevator::E2(WaitNode& node, std::uint32_t now) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;

    assert(elevator.STATE != 0 && "E2 runs only while moving");

    trace(sim, "E2", "Elevator stops");

    // 1 STATE is GOINGUP
    if (elevator.STATE > 0) {
        // Are there calls for higher floors?
        bool has_higher_call = false;
        for (std::uint32_t j = elevator.FLOOR + 1; j < FLOORS; j++) {
            if (sim.CALLS[j] != 0) {
                has_higher_call = true;
                break;
            }
        }

        // If yes, go to E3
        if (has_higher_call) {
            E3(node, now);
            return;
        }
        // Have passengers in the elevator called for lower floors?
        bool has_lower_callcar = false;
        for (std::uint32_t j = 0; j < elevator.FLOOR; j++) {
            if (sim.CALLS[j] & CALLCAR) {
                has_lower_callcar = true;
                break;
            }
        }

        // If yes, reverse direction of STATE, else set STATE to NEUTRAL
        if (has_lower_callcar) {
            elevator.STATE = -elevator.STATE;
        } else {
            elevator.STATE = 0;
        }
    }

    // 2 STATE is GOINGDOWN
    else if (elevator.STATE < 0) {
        // Are there calls for lower floors?
        bool has_lower_call = false;
        for (std::uint32_t j = 0; j < elevator.FLOOR; j++) {
            if (sim.CALLS[j] != 0) {
                has_lower_call = true;
                break;
            }
        }

        // If yes, go to E3
        if (has_lower_call) {
            E3(node, now);
            return;
        }

        // Have passengers in the elevator called for higher floors?
        bool has_higher_callcar = false;
        for (std::uint32_t j = elevator.FLOOR + 1; j < FLOORS; j++) {
            if (sim.CALLS[j] & CALLCAR) {
                has_higher_callcar = true;
                break;
            }
        }

        // If yes, reverse direction of STATE, else set STATE to NEUTRAL
        if (has_higher_callcar) {
            elevator.STATE = -elevator.STATE;
        } else {
            elevator.STATE = 0;
        }
    }
    else {
        // STATE == NEUTRAL: should not happen, caller invariant
        return;
    }

    // Set all CALL variables for the current FLOOR to zero
    sim.CALLS[elevator.FLOOR] = 0;

    // Jump to E3
    E3(node, now);
}

// [Open door]
void Elevator::E3(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;

    trace(sim, "E3", "Elevator doors start to open");

    // If activity already scheduled: remove it from WAIT list
    // Note that it can be removed as WaitList::hold unlinks the node before sorting
    // sim.cancel(elevator.ELEV3);

    // Schedule activity E9 after 300 units
    std::uint32_t delay = 300;
    sim.hold(elevator.ELEV3, delay);

    // Schedule activity E5 after 76 units
    delay = 76;
    sim.hold(elevator.ELEV2, delay);

    // Set D2 to nonzero
    elevator.D2 = 1;

    // Set D1 to nonzero
    elevator.D1 = 1;

    elevator.first_search = true;

    delay = 20;
    elevator.E4A(delay);
}

// [Let people out, in]
void Elevator::E4(WaitNode& node, std::uint32_t) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;

    // Copy first_search and set it to false in elevator
    bool first_search = elevator.first_search;
    elevator.first_search = false;

    ElevatorNode* head = &sim.ELEVATOR_LIST.head;
    ElevatorNode* C = head->left2;

    // Search ELEVATOR_LIST from tail to head
    while (C != head) {
        // Compare OUT node with FLOOR and continue if it's not equal
        if (C->OUT != elevator.FLOOR) {
            C = C->left2;
            continue;
        }

        // Otherwise prepare to send user to U6
        // Set NEXTINST of the node to U6 in place
        // Note that async/await approach can't do this
        C->set_next_inst(&Users::U6);

        // Put user at front of the WaitList (at front of the kernel)
        sim.immed(*C);

        // Wait 25 units and repeat E4A
        std::uint32_t delay = 25;
        elevator.E4A(delay);

        // Return to simulate other events
        return;
    }

    // If C == ELEVATOR_LIST.head: search is complete for ELEVATOR_LIST
    
    // Now handle the QUEUE[elevator.FLOOR] list
    head = &sim.QUEUE[elevator.FLOOR].head;
    C = head->right2;
    
    if (C != head) {
        // Cancel action U4 for this User
        sim.cancel(*C);

        // Prepare to replace U4 by U5;
        C->set_next_inst(&Users::U5);

        // Put user at front of the WaitList (at front of the kernel)
        sim.immed(*C);

        // Wait 25 units and repeat E4A
        std::uint32_t delay = 25;
        elevator.E4A(delay);

        // Return to simulate other events
        return;
    }

    // If C == QUEUE[elevator.FLOOR].head: floor is empty

    // Set D1 = 0
    elevator.D1 = 0;

    // Set D3 = nonzero
    elevator.D3 = 1;

    if (first_search) {
        trace(sim, "E4", "Doors open, nobody is there");
    }
}

void Elevator::E5(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;

    // Is D1 == 0?
    // If not, people are getting in or out
    if (elevator.D1 != 0) {
        // Print
        trace(sim, "E5", "Elevator doors flutter");

        // Wait 40 units, repeat E5
        std::uint32_t delay = 40;
        elevator.E5A(delay);
        return;
    }

    // Print
    trace(sim, "E5", "Elevator doors start to close");

    // If D1 == 0: set D3 = 0
    elevator.D3 = 0;

    // Wait 20 units, then go to E6
    std::uint32_t delay = 20;
    sim.holdc(elevator.ELEV1, delay, &E6);
}

// [Set inaction indicator]
void Elevator::E9(WaitNode& node, std::uint32_t /*now*/) noexcept {
    ElevatorSimulation& sim = begin_step(node);
    Elevator& elevator = sim.elevator;

    // Unschedule ELEV3
    // The kernel unlinked ELEV3 before running E9, 
    // so we can don't need to call: 
    // sim.cancel(elevator.ELEV3);

    // Set D2 = 0
    elevator.D2 = 0;

    // Call the decision
    sim.decision(&E9);
}
