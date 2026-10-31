#include "kernel_port.hpp"
#include "stm32g4xx_hal.h"  // Only for the RCC macro

extern "C" __attribute__((weak)) void kernel_port_after_step(void) {}
KernelStats kernel_stats __attribute__((used)) = {};

extern "C" void kernel_port_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();  // macro enables the clock and then reads the register back
    TIM2->PSC = 170 - 1;          // the prescaler: divides the input clock, so 170 MHz becomes 1 tick per µs
    TIM2->ARR = 0xFFFFFFFF;       // auto-reload: the value at which it wraps. 0xFFFFFFFF uses all 32 bits
    TIM2->EGR = TIM_EGR_UG;       // control: force an update
    TIM2->CR1 = TIM_CR1_CEN;      // control: start

    NVIC_SetPriority(PendSV_IRQn, (1u << __NVIC_PRIO_BITS) - 1);  // lowest: 15
    NVIC_SetPriority(TIM2_IRQn, 2);  // Above PendSV(15)
    NVIC_EnableIRQ(TIM2_IRQn);
}

// aim() : the alarm clock. After the loop, nothing is due now, but the next node is due at some later time. Something has to run PendSV again at that moment, and aim() arranges it:
// It writes the next deadline into TIM2->CCR1.
// When CNT reaches that value, TIM2 raises an interrupt, and the TIM2 handler pends PendSV.
// PendSV runs the due node and calls aim() again.
// Without aim(), the first PendSV runs whatever is due at start, and then nothing ever wakes the kernel again: the elevator would stop after its first steps. 
static void aim() noexcept { 
    const auto t = kernel.get_next_time();

    // Nothing waits: no alarm
    if (!t) {
        TIM2->DIER = TIM2->DIER & ~TIM_DIER_CC1IE;
        return;
    }

    // 1. Write CCR1 (the alarm: CNT == CCR1 ->interrupt)
    TIM2->CCR1 = *t;

    // 2. Clear the flag (drop a stale match from earlier alarm)
    TIM2->SR = ~TIM_SR_CC1IF;

    // 3. Enable the interrupt (interrupt on compare)
    TIM2->DIER = TIM2->DIER | TIM_DIER_CC1IE;

    // 4. Then race check
    if(!WaitList::is_less(now(), *t)) pend_kernel();
}

extern "C" void TIM2_IRQHandler(void) {
    TIM2->SR = ~TIM_SR_CC1IF;      // Clear the flag or the interrup repeats forever
    pend_kernel();
}

extern "C" void kernel_pendsv(void) {
    for(;;) {
        const uint32_t t = now();
        kernel.start_event(t);
        const auto due = kernel.get_next_time(); // the head's deadline
        const uint32_t c0 = DWT->CYCCNT;
        
        if (!kernel.step(t)) break;  // nothing due: done for now

        const uint32_t cycles = DWT->CYCCNT - c0;
        const uint32_t late = t - *due;  // µs after its deadline
        
        // kernel stats
        ++kernel_stats.steps;
        kernel_stats.busy_cycles += cycles;
        if (cycles > kernel_stats.max_cycles) kernel_stats.max_cycles = cycles;
        kernel_stats.late_sum_us += late;
        if (late > kernel_stats.max_late_us) kernel_stats.max_late_us = late;

        kernel_port_after_step();
    }

    kernel_stats.refused_ops = kernel.refused();
    kernel_stats.events_dropped = kernel.events_dropped();

    if constexpr (WAIT_LIST_STATS) {    // the WAIT list's own counters, copied for C
        const WaitListStats& w = kernel.list_stats();
        kernel_stats.sortins = w.sortins;
        kernel_stats.list_max = w.length_max;
        kernel_stats.list_sum = w.length_sum;
        kernel_stats.walk_max = w.walk_max;
        kernel_stats.walk_sum = w.walk_sum;
    }
    aim();
}
