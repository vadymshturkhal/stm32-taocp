/* clock.c - 170 MHz from the HSI (16 MHz), in registers
 *
 * The same steps HAL runs for CubeMX's SystemClock_Config
 * (HAL_PWREx_ControlVoltageScaling, HAL_RCC_OscConfig, HAL_RCC_ClockConfig),
 * without the HAL. RM0440: PWR (range 1 boost), RCC (PLL, the AHB /2 step),
 * FLASH (wait states). */
#include "clock.h"
#include "stm32g4xx.h"

void clock_170mhz(void)
{
  /* 1. Range 1 boost mode: needed above 150 MHz. VOS is range 1 at reset */
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;      /* PWR's clock on */
  (void)RCC->APB1ENR1;                      /* read back: on before PWR is written */
  PWR->CR5 &= ~PWR_CR5_R1MODE;              /* 0 = boost (reset: 1, normal) */

  /* 2. Flash: 4 wait states for 170 MHz in boost mode, before the clock goes up.
   * The caches stay on and prefetch off (reset values), as HAL_Init leaves them */
  FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_4WS;
  while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS) {}

  /* 3. PLL: HSI 16 MHz / M 4 = 4 MHz, x N 85 = 340 MHz, / R 2 = 170 MHz */
  RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSI
               | ((4u - 1u) << RCC_PLLCFGR_PLLM_Pos)   /* M - 1 */
               | (85u << RCC_PLLCFGR_PLLN_Pos)
               | (0u << RCC_PLLCFGR_PLLR_Pos)          /* 0 = /2 */
               | RCC_PLLCFGR_PLLREN;                   /* the R output feeds SYSCLK */
  RCC->CR |= RCC_CR_PLLON;
  while ((RCC->CR & RCC_CR_PLLRDY) == 0u) {}

  /* 4. SYSCLK to the PLL through AHB /2: above 80 MHz, not in one step */
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) | RCC_CFGR_HPRE_DIV2;
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
  for (volatile uint32_t i = 0; i < 100u; ++i) {}       /* at least 1 µs at 85 MHz */
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) | RCC_CFGR_HPRE_DIV1;
  /* APB1 and APB2 stay /1 (reset): TIM2 counts at 170 MHz */

  SystemCoreClock = 170000000u;
}
