#pragma once
#include <cstdint>

inline constexpr std::uint32_t FLOORS = 5;
inline constexpr std::uint32_t HOME_FLOOR = 2;
static_assert(HOME_FLOOR < FLOORS, "HOME_FLOOR must be lower than FLOORS");

inline constexpr std::uint32_t CALLUP = 0b100;
inline constexpr std::uint32_t CALLDOWN = 0b010;
inline constexpr std::uint32_t CALLCAR = 0b001;