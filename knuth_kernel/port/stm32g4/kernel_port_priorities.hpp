/*
 * kernel_port_priorities.hpp - the kernel's levels (stage 2): one Kernel, a WAIT list per NVIC priority.
 *
 * A level is an exception handler at a chosen priority, with its own WAIT list: the
 * NVIC does the preemption, and only that level's handler touches its list (rule 1).
 * Elevator context: Elevator program moves to level_14, the trace drain and tx_watchdog to level_15. 
*/

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "kernel_port.hpp"

/* 
 * The kernel levels sit below every real interrupt: TIM2 (2, the alarm) 
 * and the ISRs that post events (3) must be able to preempt any step.
*/
inline constexpr std::uint8_t first_kernel_priority = 4;

/* constexpr, fixed part of a level. */
struct LevelConfig
{
    std::uint8_t priority;  // NVIC priority, 0 = the highest
    IRQn_Type irq;          // the handler that runs it: PendSV, or a spare IRQ pended with NVIC->STIR
    std::uint8_t channel;   // its alarm in TIM2: 0 = CCR1/CC1IF, 1 = CCR2/CC2IF
};

inline constexpr std::array<LevelConfig, 2> level_config
{
    {
        { .priority = 14, .irq = CORDIC_IRQn, .channel = 0 },  // the elevator
        { .priority = 15, .irq = PendSV_IRQn, .channel = 1 },  // the trace drain, tx_watchdog
    }
};

consteval bool level_priorities_in_range() 
{
    for (const LevelConfig& c : level_config)
    {
        if (c.priority < first_kernel_priority || c.priority >= (1u << __NVIC_PRIO_BITS))
        {
            return false;
        }
    }
    return true;
}

consteval bool level_channels_exist() 
{
    for (const LevelConfig& c : level_config)
    {
        if (c.channel > 1) return false;
    }

    return true;
}

consteval bool level_nothing_shared()
{
    for (std::size_t i = 0; i < level_config.size(); ++i)
    {
        for (std::size_t j = i + 1; j < level_config.size(); ++j) 
        {
            const LevelConfig& a = level_config[i];
            const LevelConfig& b = level_config[j];
            if (a.priority == b.priority || a.irq == b.irq || a.channel == b.channel) return false;
        }
    }

    return true;

}
static_assert(level_priorities_in_range(), "level_config: a kernel level's priority must be 4..15, below the real interrupts");
static_assert(level_channels_exist(), "level_config: arm() knows channels 0 (CCR1) and 1 (CCR2) only");
static_assert(level_nothing_shared(), "level_config: two levels share a priority, an IRQ or an alarm channel");

/* A name for a missing level, like level_13, is a compile error. */
void no_kernel_level_at_this_priority();

consteval std::size_t level_index(std::uint8_t priority) 
{
    for (std::size_t i = 0; i < level_config.size(); ++i)
    {
        if (level_config[i].priority == priority) return i;
    }
    
    no_kernel_level_at_this_priority();
    
    return 0;
}

/* The kernel: every level's WAIT list. A node goes to its level's list (kernel.attach<P>);
 * only that level's handler touches the list (rule 1) */
constinit inline Kernel kernel;

/* constinit, built at compile time, written at run time. */
struct KernelLevel 
{
    KernelStats& stats;         // a C global
    const LevelConfig& config;  // its priority, IRQ and alarm channel
};

constinit inline std::array<KernelLevel, level_config.size()> levels
{
    {
        { .stats = kernel_stats_level_14, .config = level_config[level_index(14)] },
        { .stats = kernel_stats_level_15, .config = level_config[level_index(15)] },
    }
};

/* The names the programs use. Each entry above finds its config by priority,
 * so reordering level_config can't pair a name with another level's stats */
inline constexpr KernelLevel& level_14 = levels[0];  // the elevator
inline constexpr KernelLevel& level_15 = levels[1];  // the trace drain, tx_watchdog

/*
 * The doorbell: makes the level's handler pending. 
 * It runs as soon as its priority allows; several rings before it runs are one run. 
 */
inline void ring(const KernelLevel& level) noexcept 
{
    if (level.config.irq == PendSV_IRQn)
    {
        pend_kernel();  // a system exception: ICSR (STIR can't pend it)
    }
    else
    {
        NVIC->STIR = static_cast<std::uint32_t>(level.config.irq);
    }
}

/*
 * The compile-time versions, for code that knows its level's priority P: the level's
 * row of level_config is then a constant, so its channel and IRQ fold into the code.
 * The port's hot paths (the alarm, the drain loop) use these: read through levels at
 * run time, the same values cost 0.5 µs of average lateness.
 */
template <std::uint8_t P>
constexpr KernelLevel& level_at() noexcept
{
    static_assert(P == 14 || P == 15, "level_at: no kernel level at this priority");
    if constexpr (P == 14) return level_14;
    else return level_15;
}

template <std::uint8_t P>
inline void ring_at() noexcept
{
    constexpr const LevelConfig& config = level_config[level_index(P)];
    if constexpr (config.irq == PendSV_IRQn)
    {
        pend_kernel();  // a system exception: ICSR (STIR can't pend it)
    }
    else
    {
        NVIC->STIR = static_cast<std::uint32_t>(config.irq);
    }
}

