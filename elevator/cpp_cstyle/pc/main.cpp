// The C elevator's design in C++, run on the PC: the same output as c_native,
// compared with the golden traces in elevator/measurements.
//
//   ./elevator_sim [users]     (default 4, as in C)
//
// Build (from elevator/cpp_cstyle): see README.md; this file is the PC entry
// point, the elevator itself (the *.cpp next to README.md) has no main.
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "cstyle_loop.h"

int main(int argc, char** argv) {
    std::uint32_t users_quantity = 4;
    if (argc > 1) {
        users_quantity = static_cast<std::uint32_t>(std::strtoul(argv[1], nullptr, 10));
    }

#ifdef TRACE
    std::printf("TIME STATE FLOOR D1 D2 D3 step action\n");
#endif

    if (cstyle_elevator_start(users_quantity) != 0) {
        std::fprintf(stderr, "cstyle_elevator_start failed\n");
        return 1;
    }

    // C's cycle(): run the first node until the WAIT list is empty
    while (cstyle_elevator_step()) {}

    cstyle_elevator_stop();
    return 0;
}
