#pragma once
#include <array>
#include <cstdint>

/*
 * The kernel levels' NVIC priorities: the one place they are written. kernel.hpp
 * takes them from here (a WAIT list for each), and level_config's rows refer to
 * them (kernel_port_priorities.hpp). A new level: a priority here, a row there, its
 * stats global and its handler (the vector table fixes the handler's name).
 * No CMSIS in this file: kernel.hpp is the portable part.
 */
inline constexpr std::array<std::uint32_t, 2> kernel_priorities{14, 15};
