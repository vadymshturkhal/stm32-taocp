// The port needs only CMSIS (stm32g4xx.h). The HAL tick (HAL_GetTick from TIM2)
// is built only with the HAL: USE_HAL_DRIVER, which CubeMX projects define.
#include "kernel_port.hpp"
#include "kernel_port_priorities.hpp"
#include <utility>   // std::index_sequence: one alarm check per level
#ifdef USE_HAL_DRIVER
#include "stm32g4xx_hal.h"  // the HAL tick below: uwTick, HAL_SuspendTick
#endif

KernelStats kernel_stats_level_14 __attribute__((used)) = {};
KernelStats kernel_stats_level_15 __attribute__((used)) = {};

/* Clock on and set priority level */
extern "C" void kernel_port_init(void)
{
    RCC->APB1ENR1 = RCC->APB1ENR1 | RCC_APB1ENR1_TIM2EN;   // TIM2's clock on
    (void)RCC->APB1ENR1;       // read back: short delay before the first TIM2 access (the clock needs a couple of cycles to reach the peripheral)
    TIM2->PSC = 170 - 1;       // the prescaler: 170 MHz / 170 = 1 MHz tick
    TIM2->ARR = 0xFFFFFFFF;    // auto-reload: the value at which it wraps. 0xFFFFFFFF uses all 32 bits
    TIM2->EGR = TIM_EGR_UG;    // reinitialize the counter and generates an update of the registers
    TIM2->CR1 = TIM2->CR1 | TIM_CR1_CEN;  // start counting

    // The alarms: both compare interrupts on, once. arm() never writes DIER: a
    // read-modify-write there could be preempted by the other level and undo its bit
    TIM2->SR = ~(TIM_SR_CC1IF | TIM_SR_CC2IF);      // no stale match: each level arms its own
    TIM2->DIER = TIM_DIER_CC1IE | TIM_DIER_CC2IE;

    // The levels' handlers, at the priorities in level_config (PendSV is always enabled)
    for (const KernelLevel& level : levels)
    {
        NVIC_SetPriority(level.config.irq, level.config.priority);
        if (level.config.irq >= 0) NVIC_EnableIRQ(level.config.irq);
    }

    NVIC_SetPriority(TIM2_IRQn, 2);     // above every level: a running step never delays an alarm
    NVIC_EnableIRQ(TIM2_IRQn);
}

/* main's kick, once its programs are set up: from here on, only a level's handler
 * touches that level's kernel (rule 1). An empty level runs once and arms nothing */
extern "C" void kernel_port_start(void)
{
    for (const KernelLevel& level : levels) ring(level);
}

/*
 * The hot paths take the level's priority P as a template argument (level_at<P>,
 * ring_at<P> in kernel_port_priorities.hpp): its row of level_config is then known at
 * compile time, and the channel, flag and IRQ are constants in the code. Read from
 * levels at run time they cost 0.5 µs of average lateness (0.77 instead of 0.28 µs).
 */

/* Wakes level P at its next deadline */
template <std::uint8_t P>
static void arm() noexcept
{
    KernelLevel& level = level_at<P>();
    constexpr const LevelConfig& config = level_config[level_index(P)];

    // 1. Take the next time
    const auto t = level.kernel.get_next_time();

    // 2. If the list is empty: return
    if (!t) {
        return;
    }

    // 3. Otherwise write the level's compare register, chosen by its alarm channel,
    // 4. then clear only its flag with a single write to SR (rc_w0: the 1s change nothing)
    if constexpr (config.channel == 0)
    {
        TIM2->CCR1 = *t;
        TIM2->SR = ~TIM_SR_CC1IF;
    }
    else    // channel 1: level_channels_exist() rules out the others
    {
        TIM2->CCR2 = *t;
        TIM2->SR = ~TIM_SR_CC2IF;
    }

    // 5. The race check: the deadline is already here (now >= t), so ring the level
    if (!WaitList::is_less(now(), *t)) ring_at<P>();
}

/* Level P's alarm in TIM2_IRQHandler: its flag set? Clear it (or the interrupt repeats forever), ring the level */
template <std::uint8_t P>
static void check_alarm(uint32_t sr) noexcept
{
    constexpr uint32_t flag = level_config[level_index(P)].channel == 0 ? TIM_SR_CC1IF : TIM_SR_CC2IF;
    if (sr & flag)
    {
        TIM2->SR = ~flag;
        ring_at<P>();
    }
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

/* Overrides HAL's __weak HAL_GetTick: SysTick's uwTick until the switch, then TIM2 */
extern "C" uint32_t HAL_GetTick(void)
{
    return tim2_tick ? tim2_ms() + tick_offset : uwTick;
}

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

    // One check per level, expanded at compile time from level_config
    [sr]<std::size_t... I>(std::index_sequence<I...>)
    {
        (check_alarm<level_config[I].priority>(sr), ...);
    }(std::make_index_sequence<level_config.size()>{});
}

/*
 * One run of a level: every due node and every posted event, then its next alarm.
 * Only that level's handler calls it (rule 1). A higher level can preempt it between
 * or inside steps: the levels share nothing but TIM2, whose registers they split.
 * cycles include any higher level that ran inside the step.
 */
template <std::uint8_t P>
static void run_level() noexcept
{
    KernelLevel& level = level_at<P>();
    Kernel& kernel = level.kernel;
    KernelStats& stats = level.stats;

    for (;;)
    {
        const uint32_t t = now();
        kernel.start_event(t);
        const auto head_deadline = kernel.get_next_time();
        const uint32_t c0 = DWT->CYCCNT;

        if (!kernel.step(t)) break;

        const uint32_t cycles = DWT->CYCCNT - c0;
        const uint32_t late = t - *head_deadline;

        ++stats.steps;
        stats.busy_cycles += cycles;
        if (cycles > stats.max_cycles) stats.max_cycles = cycles;
        stats.late_sum_us += late;
        if (late > stats.max_late_us) stats.max_late_us = late;
    }

    stats.refused_ops = kernel.refused();
    stats.events_dropped = kernel.events_dropped();

    // the WAIT list's own counters, copied for C
    if constexpr (WAIT_LIST_STATS)
    {
        const WaitListStats& w = kernel.list_stats();
        stats.sortins = w.sortins;
        stats.list_max = w.length_max;
        stats.list_sum = w.length_sum;
        stats.walk_max = w.walk_max;
        stats.walk_sum = w.walk_sum;
    }
    arm<P>();
}

// The handlers are fixed by the vector table: check the table agrees
static_assert(level_config[level_index(14)].irq == CORDIC_IRQn, "level 14 runs in CORDIC_IRQHandler");
static_assert(level_config[level_index(15)].irq == PendSV_IRQn, "level 15 runs in PendSV_Handler");

/* Level 14 (the elevator): CORDIC's vector, pended by ring_at<14>(). The CORDIC itself stays off */
extern "C" void CORDIC_IRQHandler(void)
{
    run_level<14>();
}

/* Level 15, called from PendSV_Handler */
extern "C" void kernel_PendSV_Handler(void)
{
    run_level<15>();
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
