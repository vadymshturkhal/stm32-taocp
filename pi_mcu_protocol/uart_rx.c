/* Claude generated docstring.
 * uart_rx.c - bytes received on the link UART (LINK_HUART), one interrupt per byte
 *
 * The interrupt puts each byte into rx_buf. The main loop takes them out
 * with UART_RX_Read(). If the buffer is full, the byte is dropped and counted.
 *
 * LINK_OVER_STLINK (main.h) picks the UART:
 *   1: LPUART1, set up by BSP_COM_Init(). The BSP enables no interrupt, so
 *      UART_RX_Init() enables it and this file supplies LPUART1_IRQHandler().
 *   0: USART1, set up by CubeMX, which also enables its interrupt and
 *      generates USART1_IRQHandler() in stm32g4xx_it.c.
 */
#include "main.h"               /* LINK_HUART */
#include "uart_rx.h"
#include "uart_tx.h"            /* UART_TX_OnError() */

static uint8_t rx_byte;                             /* byte being received by the interrupt */
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];   /* ring buffer filled by the interrupt */
static volatile uint16_t rx_head = 0;               /* next write position (interrupt) */
static volatile uint16_t rx_tail = 0;               /* next read position (main loop) */
static volatile uint32_t rx_dropped = 0;            /* bytes lost: buffer full or overrun */

void UART_RX_Init(void)
{
#if LINK_OVER_STLINK
  HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);
#endif
  HAL_UART_Receive_IT(LINK_HUART, &rx_byte, 1);
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

#if LINK_OVER_STLINK
void LPUART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(LINK_HUART);
}
#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == LINK_HUART)
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
    HAL_UART_Receive_IT(LINK_HUART, &rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart == LINK_HUART)
  {
    /* Increase dropped bytes counter*/
    if (huart->ErrorCode & HAL_UART_ERROR_ORE)
    {
      rx_dropped++;
    }
    if (huart->ErrorCode & HAL_UART_ERROR_DMA)
    {
      UART_TX_OnError();                            /* the TX DMA stopped: free its line */
    }
    HAL_UART_Receive_IT(LINK_HUART, &rx_byte, 1);   /* restart reception */
  }
}
