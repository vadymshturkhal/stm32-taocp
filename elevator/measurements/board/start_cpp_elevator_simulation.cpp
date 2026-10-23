// start_cpp_elevator_simulation.cpp - one complete run of the C++ elevator, called from C
//
// Like the C start_elevator_simulation(): every call starts from a fresh state
// (a new kernel, a new simulation with its users' pool, VALUES from seed 1) and
// runs until the WAIT list is empty, so a cold and a warm run do the same work.
//
// The objects live in static storage, not on the stack: the simulation is a few
// hundred bytes and the linker script reserves only 1 KB of stack (_Min_Stack_Size).
// std::optional builds them in place for each run and destroys them after it
// (the destructor of the users' pool gives its memory back, as C's free() does).
#include <cstdint>
#include <optional>

#include "kernel.hpp"
#include "elevator_simulation.hpp"
#include "start_cpp_elevator_simulation.h"

static std::optional<Kernel> kernel;
static std::optional<ElevatorSimulation> simulation;

extern "C" std::uint32_t start_cpp_elevator_simulation(std::uint32_t max_users) {
    kernel.emplace();
    simulation.emplace(*kernel);

    // Knuth's TIME 0 is kernel tick 0, as on the PC
    simulation->start(max_users, 0);

    // Event-driven, like Knuth's CYCLE: jump to the next deadline, run every due node
    while (const auto t = kernel->get_next_time()) {
        while (kernel->step(*t)) {}
    }

    simulation.reset();     // before the kernel: the simulation holds a reference to it
    kernel.reset();
    return 0;
}

// ---- The elevator in the main loop, on the real clock ----

extern "C" void cpp_elevator_start(std::uint32_t max_users, std::uint32_t now_ms) {
    kernel.emplace();
    simulation.emplace(*kernel);
    simulation->start(max_users, now_ms);   // Knuth's TIME 0 = now
}

extern "C" int cpp_elevator_step(std::uint32_t now_ms) {
    return (kernel && kernel->step(now_ms)) ? 1 : 0;
}

// As fast as possible: no waiting for the real clock. The first node runs at its
// own time (Knuth's CYCLE: jump to the next deadline), one node per call.
extern "C" int cpp_elevator_step_next(void) {
    if (!kernel) return 0;
    const auto t = kernel->get_next_time();
    return (t && kernel->step(*t)) ? 1 : 0;
}

extern "C" int cpp_elevator_running(void) {
    return (kernel && kernel->get_next_time()) ? 1 : 0;
}

extern "C" std::uint32_t cpp_elevator_time(void) {
    return simulation ? simulation->TIME : 0;
}

extern "C" void cpp_elevator_stop(void) {
    simulation.reset();     // before the kernel: the simulation holds a reference to it
    kernel.reset();
}
