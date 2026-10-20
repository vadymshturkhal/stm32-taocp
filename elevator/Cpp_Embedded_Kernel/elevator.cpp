#include "elevator.hpp"
#include "elevator_simulation.hpp"
#include "trace.hpp"

void Elevator::E1A() noexcept {
    ELEV1.set_next_inst(&E1);
}

void Elevator::E1(WaitNode& /*C*/, std::uint32_t /*now*/) noexcept {

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
