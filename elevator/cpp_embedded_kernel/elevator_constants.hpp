#pragma once
#include <cstdint>
#include <cstddef>

inline constexpr std::uint32_t FLOORS = 5;
inline constexpr std::uint32_t HOME_FLOOR = 2;
static_assert(HOME_FLOOR < FLOORS, "HOME_FLOOR must be lower than FLOORS");

inline constexpr std::uint32_t CALLUP = 0b100;
inline constexpr std::uint32_t CALLDOWN = 0b010;
inline constexpr std::uint32_t CALLCAR = 0b001;
inline constexpr std::uint32_t ANY_CALL = CALLUP | CALLDOWN | CALLCAR;

// The kernel level (NVIC priority) the elevator's nodes run on: kernel.attach<ELEVATOR_LEVEL>
inline constexpr std::uint32_t ELEVATOR_LEVEL = 14;

// Kernel ticks per Knuth time unit. 
// Originally it is set to tenths of seconds, for that set it to 100
// 1 is the fastest speed
inline constexpr std::uint32_t TICKS_PER_TIME_UNIT = 1000;
static_assert(TICKS_PER_TIME_UNIT > 0);
inline constexpr std::uint32_t VALUES_SEED = 1;

// Users in the system at once (not in total); the most seen is 7-10, depending on VALUES
inline constexpr std::size_t POOL_SIZE = 32;

// Users in the whole run; checksum 2487227626 and 544 steps are for 66
inline constexpr std::uint32_t USERS = 66;
