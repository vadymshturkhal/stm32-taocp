/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart_tx.h"
#include "uart_rx.h"
#include "link.h"
#include "elevator_cpp/start_cpp_elevator_simulation.h"
#include "kernel_port.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
/* The C++ elevator on the Knuth kernel, in real time: the kernel runs in PendSV,
 * woken by the TIM2 compare at each deadline; main runs the link and sleeps.
 * The per-step numbers (steps, cycles, lateness) are the port's kernel_stats.
 * Globals, so they survive -O3/LTO and Live Expressions can read them while it runs. */
#define ELEVATOR_USERS 66

typedef struct
{
  uint32_t start_cycles;    /* DWT cycles of the setup (malloc, init, users_start) */
  uint32_t elapsed_us;      /* TIM2 µs from the start to finished */
  uint32_t idle_us;         /* µs main slept in __WFI meanwhile */
} ElevatorRun;

enum { ELEVATOR_PHASE_CPP, ELEVATOR_PHASE_DONE };

volatile ElevatorRun elevator_cpp __attribute__((used));
volatile uint32_t elevator_phase __attribute__((used)) = ELEVATOR_PHASE_CPP;
static uint32_t elevator_start_us;
static uint32_t elevator_dwt_overhead;          /* the cost of reading CYCCNT twice */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Set up the C++ elevator, time it, and hand it to the kernel */
static void elevator_start_cpp(void)
{
  uint32_t start = DWT->CYCCNT;
  cpp_elevator_create();            /* the slow part (the users' pool), before "now" */
  elevator_start_us = TIM2->CNT;
  cpp_elevator_start(ELEVATOR_USERS, elevator_start_us);   /* Knuth's TIME 0 = now */
  uint32_t end = DWT->CYCCNT;
  elevator_cpp.start_cycles = end - start - elevator_dwt_overhead;

  pend_kernel();                    /* from here on, only PendSV touches the kernel (rule 1) */
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

  (void)steps; (void)checksum; (void)start_cycles; (void)busy_cycles;
  (void)cycles_per_step; (void)max_cycles; (void)late_sum_us; (void)late_avg_ns; (void)max_late_us;
  (void)elapsed_us; (void)idle_us; (void)idle_permille;

  __NOP();                          /* <- breakpoint here */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  kernel_port_init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 921600;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* Here, not in USER CODE 2: this is the first user block after all the
   * generated init, and code outside USER CODE blocks is deleted when CubeMX
   * regenerates. LINK_OVER_STLINK in main.h picks the UART. */

  UART_TX_Init();                   /* DMA sending */
  UART_RX_Init();                   /* interrupt receiving */
  Link_Init();                      /* sends RST, starts the protocol */

  /* The C++ elevator beside the link, in real time: the kernel runs in PendSV at
   * each deadline (TIM2 compare), main sleeps in between. */
  DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;                  /* keep the debugger connected in __WFI */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     /* the DWT cycle counter on */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  {
    uint32_t start = DWT->CYCCNT;
    uint32_t end = DWT->CYCCNT;
    elevator_dwt_overhead = end - start;
  }

  elevator_start_cpp();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Link_Poll(HAL_GetTick());

    /* PendSV runs the elevator; main only ends it. The end is the elevator's own
     * state (users gone, dormant at E1), set by PendSV: elevator_done */
    if (elevator_finishing())
    {
      elevator_cpp.elapsed_us = TIM2->CNT - elevator_start_us;
      cpp_elevator_stop();                    /* finished: none of its nodes is in the kernel */
      elevator_phase = ELEVATOR_PHASE_DONE;
      elevator_finish();
    }

    /* Sleep until the next interrupt: the TIM2 compare (a deadline), SysTick, UART.
     * Interrupts masked around it: the check and __WFI cannot be split by an
     * interrupt (no lost wake-up), and the time counts only the sleep, because
     * the woken handler (and PendSV after it) runs at __enable_irq() */
    __disable_irq();
    if (!elevator_finishing())
    {
      uint32_t t0 = TIM2->CNT;
      __WFI();                                /* wakes on a pending interrupt even when masked */
      if (elevator_phase == ELEVATOR_PHASE_CPP)
      {
        elevator_cpp.idle_us += TIM2->CNT - t0;
      }
    }
    __enable_irq();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
