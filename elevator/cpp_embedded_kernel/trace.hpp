#pragma once

class ElevatorSimulation;

#ifdef TRACE
#include <cstdint>
#include <cstddef>
#include "circle_of_buffers.hpp"
#include "elevator_simulation.hpp"

// Table 1 style trace line: "TIME STATE FLOOR D1 D2 D3 step action"
// Matches Python's row format
struct TraceRecord {
    std::uint32_t time;
    const char* step;
    const char* fmt;
    std::uint32_t args[4];
    const char* state;
    std::uint8_t floor;
    std::uint8_t d1;
    std::uint8_t d2;
    std::uint8_t d3;
};

inline CircleOfBuffers<TraceRecord, 16> trace_circle;
std::size_t format_trace(const TraceRecord& record, char* out, std::size_t size) noexcept;
inline std::uint32_t trace_dropped = 0;

template <typename... Args>
void trace(const ElevatorSimulation& sim, const char* step, const char* fmt, Args... args) noexcept {
    static_assert(sizeof...(Args) <= 4, "A trace line takes at most 4 numbers.");
    
    if (TraceRecord* r = trace_circle.assign()) {
        const Elevator& e = sim.elevator;
        *r = {sim.TIME, step, fmt,
            {static_cast<std::uint32_t>(args)...},
            e.STATE > 0 ? "U" : e.STATE < 0 ? "D" : "N",
            static_cast<std::uint8_t>(e.FLOOR),
            static_cast<std::uint8_t>(e.D1),
            static_cast<std::uint8_t>(e.D2),
            static_cast<std::uint8_t>(e.D3)};
        trace_circle.release();
    } else {
        ++trace_dropped;
    }
}
#else
[[gnu::format(printf, 3, 4)]]
inline void trace(const ElevatorSimulation&, const char*, const char*, ...) noexcept {}
#endif
