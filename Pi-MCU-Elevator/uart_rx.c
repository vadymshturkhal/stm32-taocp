/* Claude generated docstring.
 * uart_rx.c - bytes received on LPUART1 (BSP COM1), one interrupt per byte
 *
 * The interrupt puts each byte into rx_buf. The main loop takes them out
 * with UART_RX_Read(). If the buffer is full, the byte is dropped and counted.
 */
#include "main.h"
#include "stm32g4xx_nucleo.h"   /* hcom_uart[], COM1 */
#include "uart_rx.h"

static uint8_t rx_byte;                             /* byte being received by the interrupt */
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];   /* ring buffer filled by the interrupt */
static volatile uint16_t rx_head = 0;               /* next write position (interrupt) */
static volatile uint16_t rx_tail = 0;               /* next read position (main loop) */
static volatile uint32_t rx_dropped = 0;            /* bytes lost: buffer full or overrun */

void UART_RX_Init(void)
{
  HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  HAL_UART_Receive_IT(&hcom_uart[COM1], &rx_byte, 1);
}

int UART_RX_Read(uint8_t *byte)
{
  if (rx_tail == rx_head)
  {
    return 0;
  }
  *byte = rx_buf[rx_tail];
  rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;
  return 1;
}

uint32_t UART_RX_Dropped(void)
{
  return rx_dropped;
}

void LPUART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&hcom_uart[COM1]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &hcom_uart[COM1])
  {
    uint16_t next = (rx_head + 1) % UART_RX_BUF_SIZE;
    if (next != rx_tail)
    {
      rx_buf[rx_head] = rx_byte;
      rx_head = next;
    }
    else
    {
      rx_dropped++;                                 /* buffer full: this byte is lost */
    }
    HAL_UART_Receive_IT(&hcom_uart[COM1], &rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart == &hcom_uart[COM1])
  {
    /* Increase dropped bytes counter*/
    if (huart->ErrorCode & HAL_UART_ERROR_ORE)
    {
      rx_dropped++;
    }
    HAL_UART_Receive_IT(&hcom_uart[COM1], &rx_byte, 1);   /* restart reception */
  }
}
