/* uart_tx.c - bytes sent on LPUART1 (BSP COM1) using DMA
 *
 * UART_TX_Send() copies the bytes into tx_buf and returns immediately.
 * DMA sends tx_buf to the UART in the background. When a block is done,
 * HAL_UART_TxCpltCallback() starts the next one.
 */
#include "main.h"
#include "stm32g4xx_nucleo.h"   /* hcom_uart[], COM1 */
#include "uart_tx.h"

static DMA_HandleTypeDef hdma_tx;
static volatile uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint16_t tx_head = 0;   /* where UART_TX_Send() puts the next byte */
static volatile uint16_t tx_tail = 0;   /* first byte not sent yet */
static volatile uint16_t tx_len  = 0;   /* bytes DMA is sending now, 0 = idle */

void UART_TX_Init(void)
{
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  //  hdma_tx.Instance = DMA1_Stream7;
  //  hdma_tx.Init.Channel = DMA_CHANNEL_4;
  hdma_tx.Instance                 = DMA1_Channel1;
  hdma_tx.Init.Request             = DMA_REQUEST_LPUART1_TX;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;   /* not increment peripheral destination address*/
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;    /* increment the buffer address*/
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_tx) != HAL_OK)
  {
    Error_Handler();
  }

  /* Tell the UART driver which DMA channel to use for sending */
  __HAL_LINKDMA(&hcom_uart[COM1], hdmatx, hdma_tx);

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/* Start DMA on the next waiting block if DMA is idle.
 * Called from UART_TX_Send() with interrupts off, and from the completion interrupt. */
static void tx_start(void)
{
  if (tx_len != 0 || tx_tail == tx_head)
  {
    return;                                   /* busy, or nothing to send */
  }

  /* DMA needs one continuous block: if the data wraps around,
   * send up to the end of the buffer now and the rest next time. */
  uint16_t end = (tx_head > tx_tail) ? tx_head : TX_BUF_SIZE;
  tx_len = end - tx_tail;

  if (HAL_UART_Transmit_DMA(&hcom_uart[COM1], (uint8_t *)&tx_buf[tx_tail], tx_len) != HAL_OK)
  {
    tx_len = 0;                               /* could not start: try again later */
  }
}

/* Called by HAL when a DMA block has been completely sent */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &hcom_uart[COM1])
  {
    tx_tail = (tx_tail + tx_len) % TX_BUF_SIZE;
    tx_len = 0;
    tx_start();                               /* send whatever was added meanwhile */
  }
}

void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_tx);
}

void UART_TX_Send(const char *data, int len)
{
  for (int i = 0; i < len; i++)
  {
    uint16_t next = (tx_head + 1) % TX_BUF_SIZE;
    while (next == tx_tail)                   /* buffer full: keep DMA running and wait */
    {
      __disable_irq();
      tx_start();
      __enable_irq();
    }
    tx_buf[tx_head] = (uint8_t)data[i];
    tx_head = next;
  }

  __disable_irq();                            /* the interrupt also calls tx_start() */
  tx_start();
  __enable_irq();
}
