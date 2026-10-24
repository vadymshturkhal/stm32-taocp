// start_cpp_elevator_simulation.cpp - one complete run of the C++ elevator, called from C
//
// Like the C start_elevator_simulation(): every call starts from a fresh state
// (a new simulation with its users' pool, VALUES from seed 1) and runs until the
// elevator is finished, so a cold and a warm run do the same work.
//
// The kernel is the port's shared one (kernel_port.hpp): it outlives the
// elevator, so the elevator must leave no nodes in it when it is destroyed.
// finished() says when: every user gone and the elevator dormant at E1, so
// none of its nodes is in the list.
// The simulation lives in static storage, not on the stack: it is a few hundred
// bytes and the linker script reserves only 1 KB of stack (_Min_Stack_Size).
// std::optional builds it in place for each run and destroys it after it
// (the destructor of the users' pool gives its memory back, as C's free() does).
#include <cassert>
#include <cstdint>
#include <optional>

#include "kernel_port.hpp"
#include "elevator_simulation.hpp"
#include "start_cpp_elevator_simulation.h"

static std::optional<ElevatorSimulation> simulation;

extern "C" std::uint32_t start_cpp_elevator_simulation(std::uint32_t max_users) {
    simulation.emplace(kernel);

    // Knuth's TIME 0 is kernel tick 0, as on the PC
    simulation->start(max_users, 0);

    // Event-driven, like Knuth's CYCLE: jump to the next deadline, run every due node
    while (!simulation->finished()) {
        const auto t = kernel.get_next_time();   // not finished: a node of the elevator is held
        while (kernel.step(*t)) {}
    }

    simulation.reset();     // finished: no node of the elevator is left in the kernel
    return 0;
}

// ---- The elevator in the main loop, on the real clock ----

extern "C" {
    volatile uint32_t elevator_checksum;
    volatile uint32_t elevator_done = 0;
}

extern "C" void kernel_port_after_step(void) {
	elevator_checksum = elevator_checksum * 31u + simulation->TIME;
	if (simulation->finished()) elevator_done = 1;
}

// Construction (the users' pool: malloc and every node built) is most of the
// setup, so it is split from start: read "now" after it, and USER1 is not late
extern "C" void cpp_elevator_create(void) {
    simulation.emplace(kernel);
}

extern "C" void cpp_elevator_start(std::uint32_t max_users, std::uint32_t now_ms) {
    if (!simulation) cpp_elevator_create();
    simulation->start(max_users, now_ms);   // Knuth's TIME 0 = now: USER1 is due at once
}

extern "C" int cpp_elevator_step(std::uint32_t now_ms) {
    return kernel.step(now_ms) ? 1 : 0;
}

// As fast as possible: no waiting for the real clock. The first node runs at its
// own time (Knuth's CYCLE: jump to the next deadline), one node per call.
extern "C" int cpp_elevator_step_next(void) {
    const auto t = kernel.get_next_time();
    return (t && kernel.step(*t)) ? 1 : 0;
}

extern "C" int cpp_elevator_running(void) {
    return (simulation && !simulation->finished()) ? 1 : 0;
}

extern "C" std::uint32_t cpp_elevator_time(void) {
    return simulation ? simulation->TIME : 0;
}

extern "C" void cpp_elevator_stop(void) {
    // Safe once finished(): none of the elevator's nodes is in the shared kernel.
    // TODO: stopping in the middle of a run needs the elevator to cancel its own nodes.
    assert(!simulation || simulation->finished());
    simulation.reset();
}
