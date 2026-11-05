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

static bool tim2_tick = false;           // HAL_GetTick reads TIM2 (after HAL_Tick_From_TIM2)
static uint32_t tick_offset = 0;         // keeps HAL's ms clock continuous at the switch

static volatile uint32_t base_ms = 0;    // ms in all the wraps so far
static volatile uint32_t base_rem = 0;   // their leftover µs, < 1000

static void add_wrap(uint32_t& ms, uint32_t& rem) noexcept {
    ms += 4'294'967u;
    rem += 296u;
    if (rem >= 1000u) { rem -= 1000u; ++ms; }
}

// 36 cycles; the 64-bit division took 133 (measured)
static uint32_t tim2_ms() noexcept {
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();                      // no overflow ISR between the reads

    uint32_t ms = base_ms;
    uint32_t rem = base_rem;
    const uint32_t lo = TIM2->CNT;
    if ((TIM2->SR & TIM_SR_UIF) && lo < 0x80000000u) add_wrap(ms, rem);   // overflowed, ISR not run yet

    __set_PRIMASK(primask);
    const uint32_t q = lo / 1000u;        // UMULL + LSR: the compiler's reciprocal
    const uint32_t r = lo - q * 1000u;    // MLS
    return ms + q + (r + rem >= 1000u);   // the two leftovers can make one more ms
}

// Overrides HAL's __weak HAL_GetTick: SysTick's uwTick until the switch, then TIM2
extern "C" uint32_t HAL_GetTick(void) {
    return tim2_tick ? tim2_ms() + tick_offset : uwTick;
}

// Call before the kernel starts: DIER is shared with pend_next()
extern "C" void HAL_Tick_From_TIM2(void) {
    __disable_irq();
    TIM2->SR = ~TIM_SR_UIF;                         // the UG at init set it
    TIM2->DIER = TIM2->DIER | TIM_DIER_UIE;         // count overflows (every 71.6 min)
    tick_offset = uwTick - tim2_ms();               // no jump in HAL's time
    tim2_tick = true;
    HAL_SuspendTick();                              // SysTick interrupt off: no 1 ms wake-ups
    __enable_irq();
}

extern "C" void TIM2_IRQHandler(void) {
    const uint32_t sr = TIM2->SR;
    if (sr & TIM_SR_UIF) {         // overflow: one more wrap of 2^32 µs
        __disable_irq();           // no reader between the clear and the update
        TIM2->SR = ~TIM_SR_UIF;
        uint32_t ms = base_ms;     // 1. copy the volatile pair into locals
        uint32_t rem = base_rem;
        add_wrap(ms, rem);         // 2. + one wrap, with the carry when rem passes 1000
        base_ms = ms;              // 3. write both back
        base_rem = rem;
        __enable_irq();            // PRIMASK was 0, or this ISR wouldn't be running
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
        const auto head_deadline = kernel.get_next_time();
        const uint32_t c0 = DWT->CYCCNT;
        
        if (!kernel.step(t)) break;

        const uint32_t cycles = DWT->CYCCNT - c0;
        const uint32_t late = t - *head_deadline;
        
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
