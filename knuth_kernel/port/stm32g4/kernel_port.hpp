#pragma once
#include <stdint.h>
#include "stm32g4xx.h"  // CMSIS: TIM2, RCC (no HAL TIM needed)

typedef struct {
    uint32_t steps;        // 544 expected for the elevator
    uint32_t busy_cycles;  // cycles per step
    uint32_t max_cycles;   // the longest step
    uint32_t late_sum_us;  // average lateness
    uint32_t max_late_us;  // the worst lateness
    uint32_t refused_ops;  // misuse refused by the kernel (a free node held or cancelled, a null step): 0 expected
    uint32_t events_dropped;


    // The WAIT list, with KNUTH_KERNEL_STATS (C++ compiler flag) only; Knuth: ~10 nodes, SORTIN the main cost
    uint32_t sortins;      // insertions by time (hold, holdc)
    uint32_t list_max;     // the most nodes waiting when one was sorted in
    uint32_t list_sum;     // / sortins = the average
    uint32_t walk_max;     // the most nodes one sortin passed
    uint32_t walk_sum;     // / sortins = the average
} KernelStats;

#ifdef __cplusplus
extern "C" {
#endif

extern KernelStats kernel_stats_level_14;  // defined in kernel_port.cpp; level_14's (the elevator's)
extern KernelStats kernel_stats_level_15;  // level_15's (PendSV)

void kernel_port_init(void);        // callable from main.c
void kernel_PendSV_Handler(void);   // called from PendSV_Handler
#ifdef USE_HAL_DRIVER
void HAL_Tick_From_TIM2(void);      // HAL_GetTick from TIM2, SysTick off (HAL builds only)
#endif
static inline void pend_kernel(void) { SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; }
uint32_t kernel_port_sleep(void);   // __WFI from SRAM; call with interrupts masked; returns the µs slept

#ifdef __cplusplus
}
#include "kernel.hpp"

constinit inline Kernel kernel;     // global Kernel for every program

// Two kernel levels (stage 2), not used yet: the elevator moves to level_14,
// the trace drain and tx_watchdog go to level_15. Then the global kernel goes.
inline void pend_level_14() noexcept { NVIC->STIR = CORDIC_IRQn; }   // pends CORDIC_IRQHandler
inline void pend_level_15() noexcept { pend_kernel(); }              // pends PendSV

struct KernelLevel {
    Kernel kernel;               // this level's WAIT list: only this level's handler touches it (rule 1)
    KernelStats& stats;          // a C global, so main.c, GDB and setup.sh can read it
    uint8_t channel;             // 0 = CCR1/CC1IF, 1 = CCR2/CC2IF
    void (*doorbell)() noexcept; // how to run this level from anywhere
};

constinit inline KernelLevel level_14{ .kernel = {}, .stats = kernel_stats_level_14, .channel = 0, .doorbell = pend_level_14 };
constinit inline KernelLevel level_15{ .kernel = {}, .stats = kernel_stats_level_15, .channel = 1, .doorbell = pend_level_15 };

// The kernel's clock: TIM2, 32-bit, 1 tick = 1 µs, wraps every ~71.6 min
// TIM2->CNT is already volatile (CMSIS marks it __IO)
inline uint32_t now() noexcept { return TIM2->CNT; }

inline bool kernel_post(WaitNode& node) noexcept {
    const bool posted = kernel.post(node);
    pend_kernel();
    return posted;
}
#endif
