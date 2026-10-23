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
#include "elevator/c_elevator_loop.h"
#include "elevator_cstyle/cstyle_loop.h"
#include "elevator_cpp/start_cpp_elevator_simulation.h"
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
/* The elevator in the main loop, as fast as possible (simulated time), one node
 * per pass beside the link: first the C version, then C++ on the Knuth kernel.
 * Globals, so they survive -O3/LTO and Live Expressions can read them while it runs. */
#define ELEVATOR_USERS 66

typedef struct
{
  uint32_t start_cycles;    /* DWT cycles of the setup (malloc, init, users_start) */
  uint32_t busy_cycles;     /* DWT cycles of the steps that ran */
  uint32_t steps;           /* how many nodes ran: 544 for 66 users */
  uint32_t elapsed_ms;      /* wall time from start to the end, link passes included */
  uint32_t checksum;        /* of the TIME after every step: equal for C and C++ = the same run */
} ElevatorRun;

enum { ELEVATOR_PHASE_C, ELEVATOR_PHASE_CSTYLE, ELEVATOR_PHASE_CPP, ELEVATOR_PHASE_DONE };

volatile ElevatorRun elevator_c __attribute__((used));
volatile ElevatorRun elevator_cstyle __attribute__((used));   /* C's design in C++ */
volatile ElevatorRun elevator_cpp __attribute__((used));
volatile uint32_t elevator_phase __attribute__((used)) = ELEVATOR_PHASE_C;
static uint32_t elevator_start_ms;
static uint32_t elevator_dwt_overhead;          /* the cost of reading CYCCNT twice */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
uint32_t measure_c_cpp_performance(void);   /* elevator_cpp/measure_c_cpp_performance.c */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Set up the C elevator and time it */
static void elevator_start_c(void)
{
  elevator_start_ms = HAL_GetTick();
  uint32_t start = DWT->CYCCNT;
  uint32_t status = c_elevator_start(ELEVATOR_USERS);
  uint32_t end = DWT->CYCCNT;
  elevator_c.start_cycles = end - start - elevator_dwt_overhead;
  if (status != 0)
  {
    elevator_phase = ELEVATOR_PHASE_DONE;   /* no memory: nothing to run */
  }
}

/* Set up the C-style C++ elevator (C's design, written in C++) and time it */
static void elevator_start_cstyle(void)
{
  elevator_start_ms = HAL_GetTick();
  uint32_t start = DWT->CYCCNT;
  uint32_t status = cstyle_elevator_start(ELEVATOR_USERS);
  uint32_t end = DWT->CYCCNT;
  elevator_cstyle.start_cycles = end - start - elevator_dwt_overhead;
  if (status != 0)
  {
    elevator_phase = ELEVATOR_PHASE_DONE;   /* no memory: nothing to run */
  }
}

/* Set up the C++ elevator and time it */
static void elevator_start_cpp(void)
{
  elevator_start_ms = HAL_GetTick();
  uint32_t start = DWT->CYCCNT;
  cpp_elevator_start(ELEVATOR_USERS, 0);    /* Knuth's TIME 0 = kernel tick 0 */
  uint32_t end = DWT->CYCCNT;
  elevator_cpp.start_cycles = end - start - elevator_dwt_overhead;
}

/* The end of both runs: every DWT count in one place. Breakpoint on the
 * __NOP() line to read them all in the Variables view. noinline: a real
 * function even at -O3/LTO, so the breakpoint binds; volatile: the locals are kept. */
static void __attribute__((noinline)) elevator_finish(void)
{
  volatile uint32_t c_steps = elevator_c.steps;                       /* 544 expected */
  volatile uint32_t c_start_cycles = elevator_c.start_cycles;
  volatile uint32_t c_busy_cycles = elevator_c.busy_cycles;
  volatile uint32_t c_total_cycles = elevator_c.start_cycles + elevator_c.busy_cycles;
  volatile uint32_t c_cycles_per_step = elevator_c.steps ? elevator_c.busy_cycles / elevator_c.steps : 0;
  volatile uint32_t c_elapsed_ms = elevator_c.elapsed_ms;

  volatile uint32_t cs_steps = elevator_cstyle.steps;                 /* 544 expected */
  volatile uint32_t cs_start_cycles = elevator_cstyle.start_cycles;
  volatile uint32_t cs_busy_cycles = elevator_cstyle.busy_cycles;
  volatile uint32_t cs_total_cycles = elevator_cstyle.start_cycles + elevator_cstyle.busy_cycles;
  volatile uint32_t cs_cycles_per_step = elevator_cstyle.steps ? elevator_cstyle.busy_cycles / elevator_cstyle.steps : 0;
  volatile uint32_t cs_checksum = elevator_cstyle.checksum;
  (void)cs_steps; (void)cs_start_cycles; (void)cs_busy_cycles;
  (void)cs_total_cycles; (void)cs_cycles_per_step; (void)cs_checksum;

  volatile uint32_t cpp_steps = elevator_cpp.steps;                   /* 544 expected */
  volatile uint32_t cpp_start_cycles = elevator_cpp.start_cycles;
  volatile uint32_t cpp_busy_cycles = elevator_cpp.busy_cycles;
  volatile uint32_t cpp_total_cycles = elevator_cpp.start_cycles + elevator_cpp.busy_cycles;
  volatile uint32_t cpp_cycles_per_step = elevator_cpp.steps ? elevator_cpp.busy_cycles / elevator_cpp.steps : 0;
  volatile uint32_t cpp_elapsed_ms = elevator_cpp.elapsed_ms;

  volatile uint32_t c_checksum = elevator_c.checksum;
  volatile uint32_t cpp_checksum = elevator_cpp.checksum;
  volatile uint32_t same_run = (c_checksum == cpp_checksum && c_checksum == elevator_cstyle.checksum
                                 && c_steps == cpp_steps && c_steps == elevator_cstyle.steps);   /* 1 expected */
  (void)c_checksum; (void)cpp_checksum; (void)same_run;

  (void)c_steps; (void)c_start_cycles; (void)c_busy_cycles;
  (void)c_total_cycles; (void)c_cycles_per_step; (void)c_elapsed_ms;
  (void)cpp_steps; (void)cpp_start_cycles; (void)cpp_busy_cycles;
  (void)cpp_total_cycles; (void)cpp_cycles_per_step; (void)cpp_elapsed_ms;

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

  /* Elevator DWT cycles: the C version first, then C++ on the Knuth kernel
   * (66 users each, cold and warm). Before the link starts, so no interrupt
   * from it runs during a measurement. Breakpoint on its return to read them. */
//  measure_c_cpp_performance();

  UART_TX_Init();                   /* DMA sending */
  UART_RX_Init();                   /* interrupt receiving */
  Link_Init();                      /* sends RST, starts the protocol */

  /* The elevator beside the link, as fast as possible: simulated time, not the
   * real clock (every node runs at its own time, no waiting). C first, then C++. */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     /* the DWT cycle counter on */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  {
    uint32_t start = DWT->CYCCNT;
    uint32_t end = DWT->CYCCNT;
    elevator_dwt_overhead = end - start;
  }
  elevator_start_c();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Link_Poll(HAL_GetTick());

    /* One elevator node per pass, so the link still gets every other turn.
     * Interrupts (SysTick, UART, DMA) that land inside a step are counted too. */
    if (elevator_phase == ELEVATOR_PHASE_C)
    {
      uint32_t start = DWT->CYCCNT;
      int ran = c_elevator_step();
      uint32_t end = DWT->CYCCNT;
      if (ran)
      {
        elevator_c.busy_cycles += end - start - elevator_dwt_overhead;
        elevator_c.steps++;
        elevator_c.checksum = elevator_c.checksum * 31u + c_elevator_time();
      }
      else
      {
        elevator_c.elapsed_ms = HAL_GetTick() - elevator_start_ms;
        c_elevator_stop();                    /* free the memory */
        elevator_phase = ELEVATOR_PHASE_CSTYLE;
        elevator_start_cstyle();
      }
    }
    else if (elevator_phase == ELEVATOR_PHASE_CSTYLE)
    {
      uint32_t start = DWT->CYCCNT;
      int ran = cstyle_elevator_step();
      uint32_t end = DWT->CYCCNT;
      if (ran)
      {
        elevator_cstyle.busy_cycles += end - start - elevator_dwt_overhead;
        elevator_cstyle.steps++;
        elevator_cstyle.checksum = elevator_cstyle.checksum * 31u + cstyle_elevator_time();
      }
      else
      {
        elevator_cstyle.elapsed_ms = HAL_GetTick() - elevator_start_ms;
        cstyle_elevator_stop();               /* free the memory */
        elevator_phase = ELEVATOR_PHASE_CPP;
        elevator_start_cpp();
      }
    }
    else if (elevator_phase == ELEVATOR_PHASE_CPP)
    {
      uint32_t start = DWT->CYCCNT;
      int ran = cpp_elevator_step_next();
      uint32_t end = DWT->CYCCNT;
      if (ran)
      {
        elevator_cpp.busy_cycles += end - start - elevator_dwt_overhead;
        elevator_cpp.steps++;
        elevator_cpp.checksum = elevator_cpp.checksum * 31u + cpp_elevator_time();
      }
      else
      {
        elevator_cpp.elapsed_ms = HAL_GetTick() - elevator_start_ms;
        cpp_elevator_stop();                  /* the pool gives its memory back */
        elevator_phase = ELEVATOR_PHASE_DONE;
        elevator_finish();
      }
    }
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
