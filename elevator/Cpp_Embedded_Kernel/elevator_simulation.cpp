#include <cstdint>
#include "elevator_simulation.hpp"

void ElevatorSimulation::decision(WaitNode::NextInst caller) noexcept {
    // D1. Decision necessary?
    if (elevator.STATE != 0) return;
    
    // D2. Should door open?
    if (elevator.ELEV1.get_next_inst() == &Elevator::E1 && CALLS[HOME_FLOOR] != 0) {
        // Wait 20 units of time
        std::uint32_t delay = 20;

        // Schedule E3
        holdc(elevator.ELEV1, delay, &Elevator::E3);
        return;
    }

    // D3. Any calls?
    std::int32_t j = -1;
    for (std::uint32_t i = 0; i < FLOORS; i++) {
        if (i == elevator.FLOOR) continue;
        if (CALLS[i] == 0) continue;
        j = static_cast<std::int32_t>(i);
        break;
    }

    // All CALL[j], j != FLOOR, are zero
    if (j == -1) {
        // Is caller E6B?
        // NOTE: I decided to use E6 as caller is E6, and MIX version uses E6B
        if (caller == &Elevator::E6){
            // If yes: set j = HOME_FLOOR
            j = static_cast<std::int32_t>(HOME_FLOOR);
        }
        else return;
    }

    // D4. Set STATE
    // STATE = j - FLOOR
    elevator.STATE = j - static_cast<std::int32_t>(elevator.FLOOR);

    // D5. Elevator dormant?
    // Jump if not at E1 or j == 2
    if (elevator.ELEV1.get_next_inst() != &Elevator::E1 || elevator.STATE == 0) return;

    // Otherwise schedule E6
    // Wait 20 units of time (same as in D2)
    std::uint32_t delay = 20;
    holdc(elevator.ELEV1, delay, &Elevator::E6);
}
