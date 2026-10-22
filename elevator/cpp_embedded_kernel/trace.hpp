#pragma once

class ElevatorSimulation;

#ifdef TRACE
// Table 1 style trace line: "TIME STATE FLOOR D1 D2 D3 step action"
// Matches Python's row format exactly
[[gnu::format(printf, 3, 4)]]  // tells the compiler that values for fmt start at param 4
void trace(const ElevatorSimulation& sim, const char* step, const char* fmt, ...) noexcept;
#else
[[gnu::format(printf, 3, 4)]]
inline void trace(const ElevatorSimulation&, const char*, const char*, ...) noexcept {}
#endif
