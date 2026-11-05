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

/* Wakes the kernel at the next deadline */
static void pend_next() noexcept {
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

static volatile uint32_t tim2_high = 0;   // TIM2 overflows: the upper 32 bits of a 64-bit µs clock
static bool tim2_tick = false;            // HAL_GetTick reads TIM2 (after kernel_port_tickless)
static uint32_t tick_offset = 0;          // keeps HAL's ms clock continuous at the switch

// HAL's ms clock from TIM2: the 64-bit µs count / 1000, truncated to 32 bits, so it
// wraps at 2^32 ms like uwTick. (TIM2->CNT / 1000 alone would jump back to 0 every
// 71.6 min: a false timeout in every "now - start > timeout".)
static uint32_t tim2_ms() noexcept {
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();                      // no overflow ISR between the two reads
    uint32_t hi = tim2_high;
    const uint32_t lo = TIM2->CNT;
    if ((TIM2->SR & TIM_SR_UIF) && lo < 0x80000000u) ++hi;   // overflowed, ISR not run yet
    __set_PRIMASK(primask);
    return static_cast<uint32_t>(((static_cast<uint64_t>(hi) << 32) | lo) / 1000u);
}

// Overrides HAL's __weak HAL_GetTick: SysTick's uwTick until the switch, then TIM2
extern "C" uint32_t HAL_GetTick(void) {
    return tim2_tick ? tim2_ms() + tick_offset : uwTick;
}

extern "C" void HAL_Tick_From_TIM2(void) {
    TIM2->SR = ~TIM_SR_UIF;                         // the UG at init set it
    TIM2->DIER = TIM2->DIER | TIM_DIER_UIE;         // count overflows (every 71.6 min)
    tick_offset = uwTick - tim2_ms();               // no jump in HAL's time
    tim2_tick = true;
    HAL_SuspendTick();                              // SysTick interrupt off: no 1 ms wake-ups
}

extern "C" void TIM2_IRQHandler(void) {
    const uint32_t sr = TIM2->SR;
    if (sr & TIM_SR_UIF) {                          // overflow: the 64-bit clock's upper half
        TIM2->SR = ~TIM_SR_UIF;
        tim2_high = tim2_high + 1;
    }
    if ((sr & TIM_SR_CC1IF) && (TIM2->DIER & TIM_DIER_CC1IE)) {   // a deadline (alarm armed)
        TIM2->SR = ~TIM_SR_CC1IF;                   // clear the flag or the interrupt repeats forever
        pend_kernel();
    }
}

/* Called from PendSV_Handler */
extern "C" void kernel_PendSV_Handler(void) {
    for(;;) {
        const uint32_t t = now();
        kernel.start_event(t);
        const auto event_time = kernel.get_next_time();
        const uint32_t c0 = DWT->CYCCNT;
        
        if (!kernel.step(t)) break;

        const uint32_t cycles = DWT->CYCCNT - c0;
        const uint32_t late = t - *event_time;  // µs after its deadline
        
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
    pend_next();
}
