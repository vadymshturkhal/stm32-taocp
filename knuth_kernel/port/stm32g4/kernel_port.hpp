#pragma once
#include <stdint.h>
#include "stm32g4xx.h"  // CMSIS: TIM2, RCC (no HAL TIM needed)

typedef struct {
    uint32_t steps;        // 544 expected for the elevator
    uint32_t busy_cycles;  // cycles per step
    uint32_t max_cycles;   // the longest step
    uint32_t late_sum_us;  // average lateness
    uint32_t max_late_us;  // the worst lateness

} KernelStats;

#ifdef __cplusplus
extern "C" {
#endif

extern KernelStats kernel_stats;    // defined in kernel_port.cpp

void kernel_port_init(void);        // callable from main.c
void kernel_pendsv(void);           // called from PendSV_Handler
void kernel_port_after_step(void);  // defined by the application, called every step
static inline void pend_kernel(void) { SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; }

#ifdef __cplusplus
}
#include "kernel.hpp"

constinit inline Kernel kernel;     // global Kernel for every program

// The kernel's clock: TIM2, 32-bit, 1 tick = 1 µs, wraps every ~71.6 min
// TIM2->CNT is already volatile (CMSIS marks it __IO), so the compiler won't cache it
inline uint32_t now() noexcept { return TIM2->CNT; }

#endif
