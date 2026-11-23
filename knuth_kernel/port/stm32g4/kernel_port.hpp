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
void kernel_port_start(void);       // main's kick, once its programs are set up: rings every level
void kernel_PendSV_Handler(void);   // called from PendSV_Handler: runs level 15
#ifdef USE_HAL_DRIVER
void HAL_Tick_From_TIM2(void);      // HAL_GetTick from TIM2, SysTick off (HAL builds only)
#endif
static inline void pend_kernel(void) { SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; }
uint32_t kernel_port_sleep(void);   // __WFI from SRAM; call with interrupts masked; returns the µs slept

#ifdef __cplusplus
}
#include "kernel.hpp"

// The kernels, one per level, and kernel_post(): kernel_port_priorities.hpp

// The kernel's clock: TIM2, 32-bit, 1 tick = 1 µs, wraps every ~71.6 min
// TIM2->CNT is already volatile (CMSIS marks it __IO)
inline uint32_t now() noexcept { return TIM2->CNT; }
#endif
