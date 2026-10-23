#pragma once
#include <cstdint>

inline constexpr std::uint32_t FLOORS = 5;
inline constexpr std::uint32_t HOME_FLOOR = 2;
static_assert(HOME_FLOOR < FLOORS, "HOME_FLOOR must be lower than FLOORS");

inline constexpr std::uint32_t CALLUP = 0b100;
inline constexpr std::uint32_t CALLDOWN = 0b010;
inline constexpr std::uint32_t CALLCAR = 0b001;
inline constexpr std::uint32_t ANY_CALL = CALLUP | CALLDOWN | CALLCAR;

// Kernel ticks per Knuth time unit. 
// Originally it is set to tenths of seconds, for that set it to 100
// 1 is the fastest speed
inline constexpr std::uint32_t TICKS_PER_TIME_UNIT = 1;
static_assert(TICKS_PER_TIME_UNIT > 0);
inline constexpr std::uint32_t VALUES_SEED = 1;
