// The elevator on the Knuth kernel, run on the PC: the same output as c_native,
// compared with the golden traces in elevator/measurements.
//
//   ./elevator_sim [users]     (default 4, as in C)
//
// Build (from elevator/cpp_embedded_kernel): see README.md; this file is the
// PC entry point, the elevator itself (the *.cpp next to README.md) has no main.
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "elevator_simulation.hpp"

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
        while (kernel.step(*t)) {}
    }

    return 0;
}
