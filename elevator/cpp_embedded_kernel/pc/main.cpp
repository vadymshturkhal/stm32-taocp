// The elevator on the Knuth kernel, run on the PC: the same output as c_native,
// compared with the golden traces in elevator/measurements.
//
//   ./elevator_sim [users]     (default 4, as in C)
//
// Build (from elevator/cpp_embedded_kernel): see README.md; this file is the
// PC entry point, the elevator itself (the *.cpp next to README.md) has no main.
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "elevator_simulation.hpp"
#include "trace.hpp"

// ElevatorSimulation::holdc with a literal 0 or nullptr as the step does not compile:
// it would hold the node with no step, and the elevator would stop silently.
// Checked here because CI builds this file. (Concepts on the class, so a call that
// does not compile makes them false instead of stopping the build.)
template <class S> concept holdc_0       = requires(S& s, WaitNode& n) { s.holdc(n, 20u, 0); };
template <class S> concept holdc_nullptr = requires(S& s, WaitNode& n) { s.holdc(n, 20u, nullptr); };
template <class S> concept holdc_step    = requires(S& s, WaitNode& n) { s.holdc(n, 20u, &Elevator::E3); };
static_assert(!holdc_0<ElevatorSimulation> && !holdc_nullptr<ElevatorSimulation>,
              "ElevatorSimulation::holdc(node, delay, 0) must not compile");
static_assert(holdc_step<ElevatorSimulation>, "ElevatorSimulation::holdc with a real step must compile");

static Kernel kernel;
static ElevatorSimulation simulation{kernel};

int main(int argc, char** argv) {
    std::uint32_t users_quantity = 4;
    if (argc > 1) {
        users_quantity = static_cast<std::uint32_t>(std::strtoul(argv[1], nullptr, 10));
    }

#ifdef TRACE
    std::printf("TIME STATE FLOOR D1 D2 D3 step action\n");
#endif

    // Knuth's TIME 0 is kernel tick 0
    simulation.start(users_quantity, 0);

    // Event-driven, like Knuth's CYCLE: jump to the next deadline, run every due node.
    // The WAIT list empties after the last user leaves and the elevator is dormant at E1.
    while (const auto t = kernel.get_next_time()) {
        while (kernel.step(*t)) {
#ifdef TRACE
            // The same path as the board: each step's records become lines afterwards
            while (TraceRecord* r = trace_circle.start_operation()) {
                char line[128];
                std::fwrite(line, 1, format_trace(*r, line, sizeof line), stdout);
                trace_circle.operation_is_completed();
            }
#endif
        }
    }

    // Misuse that a release build would refuse silently (a free node held or
    // cancelled, a null step, a bad push): a correct run has none, so CI fails on it
    const std::uint32_t refused_kernel = kernel.refused();
    const std::uint32_t refused_pool = simulation.users.pool_refused();
    if (refused_kernel != 0 || refused_pool != 0) {
        std::fprintf(stderr, "refused operations: kernel %" PRIu32 ", pool %" PRIu32 "\n",
                     refused_kernel, refused_pool);
        return 1;
    }
#ifdef TRACE
    if (trace_dropped != 0) {
        std::fprintf(stderr, "trace records dropped: %" PRIu32 "\n", trace_dropped);
        return 1;
    }
#endif

    return 0;
}
