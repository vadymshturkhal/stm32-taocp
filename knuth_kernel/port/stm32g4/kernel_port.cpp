// The port needs only CMSIS (stm32g4xx.h). The HAL tick (HAL_GetTick from TIM2)
// is built only with the HAL: USE_HAL_DRIVER, which CubeMX projects define.
#include "kernel_port.hpp"
#ifdef USE_HAL_DRIVER
#include "stm32g4xx_hal.h"  // the HAL tick below: uwTick, HAL_SuspendTick
#endif

KernelStats kernel_stats_level_14 __attribute__((used)) = {};
KernelStats kernel_stats_level_15 __attribute__((used)) = {};

/* Clock on and set priority level */
extern "C" void kernel_port_init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;   // TIM2's clock on
    (void)RCC->APB1ENR1;          // read back: short delay before the first TIM2 access (the clock needs a couple of cycles to reach the peripheral)
    TIM2->PSC = 170 - 1;          // the prescaler: 170 MHz / 170 = 1 MHz tick
    TIM2->ARR = 0xFFFFFFFF;       // auto-reload: the value at which it wraps. 0xFFFFFFFF uses all 32 bits
    TIM2->EGR = TIM_EGR_UG;       // reloads PSC/ARR without setting UIF (update interrupt flag)
    TIM2->CR1 |= TIM_CR1_CEN;     // start counting

    NVIC_SetPriority(PendSV_IRQn, (1u << __NVIC_PRIO_BITS) - 1);  // lowest: 15
    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/* Wakes the kernel at the next deadline */
static void pend_next() noexcept
{
    const auto t = kernel.get_next_time();

    // Nothing waits
    if (!t) {
        TIM2->DIER &= ~TIM_DIER_CC1IE;
        return;
    }

    // 1. Write CCR1 (the alarm: CNT == CCR1 ->interrupt)
    TIM2->CCR1 = *t;

    // 2. Clear the flag (drop a stale match from earlier alarm)
    TIM2->SR = ~TIM_SR_CC1IF;

    // 3. Update interrupt enable
    TIM2->DIER |= TIM_DIER_CC1IE;

    // 4. Go to PendSV
    if(!WaitList::is_less(now(), *t)) pend_kernel();
}

#ifdef USE_HAL_DRIVER
// The HAL tick: a ms clock from TIM2 (µs) plus its overflow count, so SysTick can stop
static bool tim2_tick = false;           // HAL_GetTick reads TIM2 (after HAL_Tick_From_TIM2)
static uint32_t tick_offset = 0;         // keeps HAL's ms clock continuous at the switch

static volatile uint32_t base_ms = 0;    // ms in all the wraps so far
static volatile uint32_t base_rem = 0;   // their leftover µs, < 1000

/*
 * Based on TAOCP Vol 2.(4.3.1) Algorithm A (Addition of nonnegative integers).
 */
static void add_wrap(uint32_t& ms, uint32_t& rem) noexcept
{
    ms += 4'294'967u;
    rem += 296u;
    if (rem >= 1000u) { rem -= 1000u; ++ms; }
}

/*
 * 36 cycles; the 64-bit division took 133 (measured)
 */ 
static uint32_t tim2_ms() noexcept
{
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();                      // no overflow ISR between the reads

    uint32_t ms = base_ms;
    uint32_t rem = base_rem;
    const uint32_t lo = TIM2->CNT;
    // UIF set: TIM2 wrapped, and the ISR (masked here) hasn't counted it yet.
    // lo small: the wrap came before the CNT read, so lo needs it added.
    // lo large: the wrap came between the CNT and SR reads; lo is from before it
    if ((TIM2->SR & TIM_SR_UIF) && lo < 0x80000000u) add_wrap(ms, rem);

    __set_PRIMASK(primask);
    const uint32_t q = lo / 1000u;        // UMULL + LSR: the compiler's reciprocal
    const uint32_t r = lo - q * 1000u;    // MLS
    return ms + q + (r + rem >= 1000u);   // the two leftovers can make one more ms
}

// Overrides HAL's __weak HAL_GetTick: SysTick's uwTick until the switch, then TIM2
extern "C" uint32_t HAL_GetTick(void)
{
    return tim2_tick ? tim2_ms() + tick_offset : uwTick;
}

// Call before the kernel starts: DIER is shared with pend_next()
extern "C" void HAL_Tick_From_TIM2(void)
{
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    TIM2->SR = ~TIM_SR_UIF;
    TIM2->DIER = TIM2->DIER | TIM_DIER_UIE;
    tick_offset = uwTick - tim2_ms();
    tim2_tick = true;
    HAL_SuspendTick();
    __set_PRIMASK(primask);
}
#endif  // USE_HAL_DRIVER

extern "C" void TIM2_IRQHandler(void)
{
    const uint32_t sr = TIM2->SR;
#ifdef USE_HAL_DRIVER
    // overflow: one more wrap of 2^32 µs
    if (sr & TIM_SR_UIF)
    {
        const uint32_t primask = __get_PRIMASK();
        __disable_irq();           // no reader between the clear and the update
        TIM2->SR = ~TIM_SR_UIF;
        uint32_t ms = base_ms;     // 1. copy the volatile pair into locals
        uint32_t rem = base_rem;
        add_wrap(ms, rem);         // 2. + one wrap, with the carry when rem passes 1000
        base_ms = ms;              // 3. write both back
        base_rem = rem;
        __set_PRIMASK(primask);    // restore, as tim2_ms() does: no reasoning about entry needed
    }
#endif

    if ((sr & TIM_SR_CC1IF) && (TIM2->DIER & TIM_DIER_CC1IE))
    {   // a deadline (alarm armed)
        TIM2->SR = ~TIM_SR_CC1IF;                   // clear the flag or the interrupt repeats forever
        pend_kernel();
    }
}

/* Called from PendSV_Handler */
extern "C" void kernel_PendSV_Handler(void)
{
    for(;;)
    {
        const uint32_t t = now();
        kernel.start_event(t);
        const auto head_deadline = kernel.get_next_time();
        const uint32_t c0 = DWT->CYCCNT;
        
        if (!kernel.step(t)) break;

        const uint32_t cycles = DWT->CYCCNT - c0;
        const uint32_t late = t - *head_deadline;
        
        // kernel stats
        ++kernel_stats_level_14.steps;
        kernel_stats_level_14.busy_cycles += cycles;
        if (cycles > kernel_stats_level_14.max_cycles) kernel_stats_level_14.max_cycles = cycles;
        kernel_stats_level_14.late_sum_us += late;
        if (late > kernel_stats_level_14.max_late_us) kernel_stats_level_14.max_late_us = late;
    }

    kernel_stats_level_14.refused_ops = kernel.refused();
    kernel_stats_level_14.events_dropped = kernel.events_dropped();

    // the WAIT list's own counters, copied for C
    if constexpr (WAIT_LIST_STATS)
    {
        const WaitListStats& w = kernel.list_stats();
        kernel_stats_level_14.sortins = w.sortins;
        kernel_stats_level_14.list_max = w.length_max;
        kernel_stats_level_14.list_sum = w.length_sum;
        kernel_stats_level_14.walk_max = w.walk_max;
        kernel_stats_level_14.walk_sum = w.walk_sum;
    }
    pend_next();
}

/*
 * Runs from SRAM: .data_ramfunc is matched by *(.data*), so startup copies it with
 * .data. With DBG_SLEEP set, the G431 can run a wrong instruction from the I-cache
 * after the wake (ES0431 2.2.7, seen even with its ISB and FLASHSMEN workarounds).
 * Call with interrupts masked; returns the µs slept.
 * 
 * Explanations:
 * .data_ramfunc, not .data.x: the .data. prefix makes gas warn "setting incorrect section attributes".
 * noinline: this keeps the sequence in SRAM. Inlined into main, it would run from flash again.
 */
extern "C" [[gnu::section(".data_ramfunc"), gnu::noinline]]
uint32_t kernel_port_sleep(void)
{
    const uint32_t t0 = now();
    __DSB();    // finish every memory access before sleeping
    __WFI();    // wakes on a pending interrupt even when masked
    __ISB();    // the code after WFI runs only after the wake
    return now() - t0;
}
