#include <cassert>
#include "elevator.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"

void Elevator::E1A() noexcept { ELEV1.set_next_inst(&E1); }
void Elevator::E1(WaitNode& /*node*/, std::uint32_t /*now*/) noexcept {}

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

void Elevator::E3(WaitNode& node, std::uint32_t /*now*/) noexcept {

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
        uint32_t delay = 40;
        elevator.E5A(delay);
        return;
    }

    // Print
    trace(sim, "E5", "Elevator doors start to close");

    // If D1 == 0: set D3 = 0
    elevator.D3 = 0;

    // Wait 20 units, then go to E6
    uint32_t delay = 20;
    sim.holdc(elevator.ELEV1, delay, Elevator::E6);
}
