/* main.c - the elevator on the Knuth kernel, CMSIS only (no HAL, no CubeMX)
 *
 * The startup file runs SystemInit (FPU on) and the C++ constructors, then main. */
#include "stm32g4xx.h"
#include "clock.h"
#include "board_main.h"
#include "stack_guard.h"

int main(void)
{
  stack_guard_init();               /* first: the MPU guard at the stack's bottom, the stack painted */
  clock_170mhz();
  NVIC_SetPriorityGrouping(3u);     /* 4 bits preemption, no sub-priority: as HAL_Init */
  board_main();                     /* never returns */
}
