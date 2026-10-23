// trace.cpp - trace.c ported 1:1 (only with -DTRACE, i.e. on the PC; "\n" like the golden traces)
#ifdef TRACE
#include <cstdarg>
#include <cstdio>
#include "cstyle.hpp"

namespace cstyle {

void trace(SharedState* shared_state, const char* step, const char* fmt, ...) {
    Elevator* elevator = shared_state->elevator;
    const char* state = elevator->STATE > 0 ? "U" : elevator->STATE < 0 ? "D" : "N";

    std::printf("%04u   %-5s %-4u %-2u %-2u %-2u  %s ",
                static_cast<unsigned>(shared_state->TIME), state, static_cast<unsigned>(elevator->FLOOR),
                static_cast<unsigned>(elevator->D1), static_cast<unsigned>(elevator->D2),
                static_cast<unsigned>(elevator->D3), step);

    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);

    std::printf("\n");
}

}  // namespace cstyle
#endif
