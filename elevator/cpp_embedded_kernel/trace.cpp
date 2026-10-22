#ifdef TRACE
#include <cstdarg>
#include <cstdio>
#include "trace.hpp"
#include "elevator_simulation.hpp"

void trace(const ElevatorSimulation& sim, const char* step, const char* fmt, ...) noexcept {
    const Elevator& elev = sim.elevator;
    const char* state = elev.STATE > 0 ? "U" : elev.STATE < 0 ? "D" : "N";

    std::printf("%04u   %-5s %-4u %-2u %-2u %-2u  %s ",
           sim.TIME, state, elev.FLOOR, elev.D1, elev.D2, elev.D3, step);

    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);
    std::printf("\n");
}
#endif
