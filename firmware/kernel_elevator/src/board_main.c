/* board_main.c - the C++ elevator on the Knuth kernel, in real time
 *
 * The kernel runs in PendSV, woken by the TIM2 compare at each deadline; main
 * only ends the run and sleeps. The per-step numbers (steps, cycles, lateness)
 * are the port's kernel_stats. Globals, so they survive -O2/LTO and Live
 * Expressions can read them while it runs.
 *
 * CMSIS only, apart from the HAL tick switch (USE_HAL_DRIVER: CubeMX builds).
 * PendSV_Handler is defined here: in a CubeMX project, NVIC -> Code generation
 * -> PendSV, untick "Generate IRQ handler", or the link finds two of them. */
#include "board_main.h"
#include "kernel_port.hpp"
#include "start_cpp_elevator_simulation.h"

#define ELEVATOR_USERS 66

typedef struct
{
  uint32_t start_cycles;    /* DWT cycles of the setup (malloc, init, users_start) */
  uint32_t elapsed_us;      /* TIM2 µs from the start to finished */
  uint32_t idle_us;         /* µs main slept in __WFI meanwhile */
  uint32_t pool_refused;    /* pushes the users' pool refused: 0 expected */
  uint32_t wakes;           /* how many times __WFI woke up */
} ElevatorRun;

enum { ELEVATOR_PHASE_CPP, ELEVATOR_PHASE_DONE };

volatile ElevatorRun elevator_cpp __attribute__((used));
volatile uint32_t elevator_phase __attribute__((used)) = ELEVATOR_PHASE_CPP;
static uint32_t elevator_start_us;
static uint32_t elevator_dwt_overhead;          /* the cost of reading CYCCNT twice */

/* PendSV runs the kernel */
void PendSV_Handler(void)
{
  kernel_PendSV_Handler();
}

/* Set up the C++ elevator, time it, and hand it to the kernel */
static void elevator_start_cpp(void)
{
  uint32_t start = DWT->CYCCNT;
  cpp_elevator_create();            /* the slow part (the users' pool), before "now" */
  elevator_start_us = TIM2->CNT;
  cpp_elevator_start(ELEVATOR_USERS, elevator_start_us);   /* Knuth's TIME 0 = now */
  uint32_t end = DWT->CYCCNT;
  elevator_cpp.start_cycles = end - start - elevator_dwt_overhead;
}

/* main's only elevator work: the end of the run */
static inline int elevator_finishing(void)
{
  return elevator_phase == ELEVATOR_PHASE_CPP && elevator_done;
}

/* The end of the run: every DWT count in one place. Breakpoint on the
 * __NOP() line to read them all in the Variables view. noinline: a real
 * function even at -O3/LTO, so the breakpoint binds; volatile: the locals are kept. */
static void __attribute__((noinline)) elevator_finish(void)
{
  volatile uint32_t steps = kernel_stats.steps;                       /* 544 expected */
  volatile uint32_t checksum = elevator_checksum;                     /* 2487227626 expected */
  volatile uint32_t start_cycles = elevator_cpp.start_cycles;
  volatile uint32_t busy_cycles = kernel_stats.busy_cycles;
  volatile uint32_t cycles_per_step = steps ? busy_cycles / steps : 0;
  volatile uint32_t max_cycles = kernel_stats.max_cycles;             /* the longest step */
  volatile uint32_t late_sum_us = kernel_stats.late_sum_us;
  volatile uint32_t late_avg_ns = steps ? (uint32_t)((uint64_t)late_sum_us * 1000u / steps) : 0;   /* < 1 µs: in ns */
  volatile uint32_t max_late_us = kernel_stats.max_late_us;           /* the worst lateness */
  volatile uint32_t elapsed_us = elevator_cpp.elapsed_us;
  volatile uint32_t idle_us = elevator_cpp.idle_us;
  volatile uint32_t idle_permille = elapsed_us ? (uint32_t)((uint64_t)idle_us * 1000u / elapsed_us) : 0;
  volatile uint32_t wakes = elevator_cpp.wakes;
  volatile uint32_t awake_us = elapsed_us - idle_us;

  /* Misuse a release build refuses silently: both must be 0 */
  volatile uint32_t refused_ops = kernel_stats.refused_ops;           /* the kernel: a free node held or cancelled, a null step */
  volatile uint32_t pool_refused = elevator_cpp.pool_refused;         /* the users' pool: a bad push */

  /* The WAIT list (with KNUTH_KERNEL_STATS in the C++ symbols; otherwise 0) */
  volatile uint32_t sortins = kernel_stats.sortins;                   /* 568 expected */
  volatile uint32_t list_max = kernel_stats.list_max;                 /* 7 */
  volatile uint32_t list_avg_x100 = sortins ? kernel_stats.list_sum * 100u / sortins : 0;   /* 310 = 3.10 nodes */
  volatile uint32_t walk_max = kernel_stats.walk_max;                 /* 6 */
  volatile uint32_t walk_avg_x100 = sortins ? kernel_stats.walk_sum * 100u / sortins : 0;   /* 194 = 1.94 nodes */

  (void)steps; (void)checksum; (void)start_cycles; (void)busy_cycles;
  (void)cycles_per_step; (void)max_cycles; (void)late_sum_us; (void)late_avg_ns; (void)max_late_us;
  (void)elapsed_us; (void)idle_us; (void)idle_permille; (void)wakes; (void)awake_us;
  (void)refused_ops; (void)pool_refused;
  (void)sortins; (void)list_max; (void)list_avg_x100; (void)walk_max; (void)walk_avg_x100;

  __NOP();                          /* <- breakpoint here */
}

void board_main(void)
{
  kernel_port_init();               /* TIM2 as the 1 µs clock, PendSV and TIM2 priorities */

  DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;                  /* keep the debugger connected in __WFI */
  DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM2_STOP;  /* TIM2 stops while the debugger halts the CPU */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     /* the DWT cycle counter on */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  {
    uint32_t start = DWT->CYCCNT;
    uint32_t end = DWT->CYCCNT;
    elevator_dwt_overhead = end - start;
  }

#ifdef USE_HAL_DRIVER
  HAL_Tick_From_TIM2();             /* HAL's ms from TIM2, SysTick off: no 1 ms wake-ups */
#endif
  elevator_start_cpp();
  pend_kernel();                    /* from here on, only PendSV touches the kernel (rule 1) */

  for (;;)
  {
    /* PendSV runs the elevator; main only ends it. The end is the elevator's own
     * state (users gone, dormant at E1), set by PendSV: elevator_done. */
    if (elevator_finishing())
    {
      elevator_cpp.elapsed_us = TIM2->CNT - elevator_start_us;
      elevator_cpp.pool_refused = cpp_elevator_pool_refused();   /* before stop: the pool goes with it */
      cpp_elevator_stop();                  /* finished: none of its nodes is in the kernel */
      elevator_phase = ELEVATOR_PHASE_DONE;
      elevator_finish();
    }

    /* Sleep until the next interrupt (the TIM2 compare: a deadline).
     * Interrupts masked around it: the check and __WFI cannot be split by an
     * interrupt (no lost wake-up), and the time counts only the sleep, because
     * the woken handler (and PendSV after it) runs at __enable_irq() */
    __disable_irq();
    if (!elevator_finishing())
    {
      uint32_t t0 = TIM2->CNT;
      __DSB();   /* finish every memory access before sleeping */
      __WFI();  /* wakes on a pending interrupt even when masked */
      __ISB();   /* flush the pipeline: the code after WFI runs only after the wake */
      if (elevator_phase == ELEVATOR_PHASE_CPP)
      {
        elevator_cpp.idle_us += TIM2->CNT - t0;
        elevator_cpp.wakes++;
      }
    }
    __enable_irq();
  }
}
