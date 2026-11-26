#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

class ElevatorSimulation;

/*
 * A trace line's action: text with a "{}" for each number, as std::format writes it.
 * Checked while the call compiles, with or without TRACE: TraceFormat<N>'s constructor
 * is consteval, so it runs on the string literal and stops the build when the "{}"
 * count isn't N, the number of arguments (std::format_string works the same way).
 */
consteval std::size_t trace_placeholders(const char* s)
{
    std::size_t n = 0;
    for (; *s; ++s)
    {
        if (s[0] == '{' && s[1] == '}')
        {
            ++n;
            ++s;
        }
    }
    return n;
}

void trace_format_does_not_match_its_arguments();   // not constexpr: reaching it stops the build, and names the error

template <std::size_t N>
struct TraceFormat
{
    const char* text;

    consteval TraceFormat(const char* s) : text(s)
    {
        if (trace_placeholders(s) != N) trace_format_does_not_match_its_arguments();
    }
};

// What a trace line's numbers may be: at most 4, each an unsigned integer of 32 bits or fewer
template <typename... Args>
concept TraceNumbers = sizeof...(Args) <= 4
    && ((std::unsigned_integral<Args> && sizeof(Args) <= sizeof(std::uint32_t)) && ...);

#ifdef TRACE
#include "circle_of_buffers.hpp"
#include "elevator_simulation.hpp"

// Table 1 style trace line: "TIME STATE FLOOR D1 D2 D3 step action"
// Matches Python's row format
struct TraceRecord
{
    std::uint32_t time;
    const char* step;
    const char* fmt;            // the action: text and a "{}" per number in args, checked by TraceFormat
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

/* type_identity_t: Args come from the numbers alone;
 * then the literal converts to the TraceFormat of that many "{}",
 * which is where the consteval check runs
 */
template <typename... Args>
    requires TraceNumbers<Args...>
void trace(const ElevatorSimulation& sim, const char* step,
           std::type_identity_t<TraceFormat<sizeof...(Args)>> fmt, Args... args) noexcept
{
    if (TraceRecord* r = trace_circle.assign())
    {
        const Elevator& e = sim.elevator;
        *r = {sim.TIME, step, fmt.text,
            {static_cast<std::uint32_t>(args)...},
            e.STATE > 0 ? "U" : e.STATE < 0 ? "D" : "N",
            static_cast<std::uint8_t>(e.FLOOR),
            static_cast<std::uint8_t>(e.D1),
            static_cast<std::uint8_t>(e.D2),
            static_cast<std::uint8_t>(e.D3)};
        trace_circle.release();
    }
    else
    {
        ++trace_dropped;
    }
}
#else
// The same signature, so the formats are checked without TRACE too
template <typename... Args>
    requires TraceNumbers<Args...>
inline void trace(const ElevatorSimulation&, const char*,
                  std::type_identity_t<TraceFormat<sizeof...(Args)>>, Args...) noexcept {}
#endif
