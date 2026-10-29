/* uart_tx.cpp - lines sent on the link UART (LINK_HUART) using DMA
 *
 * Each line goes into one buffer of a circle of buffers (TAOCP 1.4.4):
 * UART_TX_Send() fills a buffer and releases it, DMA sends one whole buffer at
 * a time, and HAL_UART_TxCpltCallback() gives it back and starts the next one.
 * Nothing waits: a full circle refuses the line (counted in UART_TX_Dropped()).
 *
 * LINK_OVER_STLINK (main.h) picks the UART:
 *   1: LPUART1. CubeMX knows nothing about its DMA, so UART_TX_Init() sets up
 *      DMA1 channel 2 and this file supplies DMA1_Channel2_IRQHandler().
 *      (Channel 1 belongs to USART1 in CubeMX.)
 *   0: USART1. CubeMX sets up its DMA (hdma_usart1_tx, DMA1 channel 1), links
 *      it to huart1 and generates DMA1_Channel1_IRQHandler().
 */
#include <cstring>
#include <cstdint>
#include "main.h"                  /* LINK_HUART, LINK_OVER_STLINK, HAL */
#include "uart_tx.h"
#include "circle_of_buffers.hpp"

// private to the file instead of a static on every line
namespace {
struct TxLine {
    std::uint8_t  data[UART_TX_LINE_SIZE];
    std::uint16_t len;
};
static_assert(UART_TX_LINE_SIZE <= UINT16_MAX, "len is 16 bits");

CircleOfBuffers<TxLine, 3> tx;
std::uint32_t tx_dropped = 0;
#if LINK_OVER_STLINK
DMA_HandleTypeDef hdma_link_tx;    // LPUART1 TX on DMA1 channel 2
#endif

// Start DMA on the oldest red line if the device is idle.
// main and the TX-complete interrupt both call it; DEVICE_BUFFER keeps them apart.
void tx_start() {
    if (TxLine* buffer = tx.start_operation()) {
        if (HAL_UART_Transmit_DMA(LINK_HUART, buffer->data, buffer->len) != HAL_OK) {
            tx.cancel_operation();  // HAL_BUSY: stays first, retried next time
        }
    }
}
}  // namespace ends

void UART_TX_Init(void)
{
#if LINK_OVER_STLINK
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_link_tx.Instance                 = DMA1_Channel2;
    hdma_link_tx.Init.Request             = DMA_REQUEST_LPUART1_TX;
    hdma_link_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_link_tx.Init.PeriphInc           = DMA_PINC_DISABLE;   /* always the UART data register */
    hdma_link_tx.Init.MemInc              = DMA_MINC_ENABLE;    /* walk through the line */
    hdma_link_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_link_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_link_tx.Init.Mode                = DMA_NORMAL;
    hdma_link_tx.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_link_tx) != HAL_OK)
    {
        Error_Handler();
    }

    /* Tell the UART driver which DMA channel to use for sending */
    __HAL_LINKDMA(LINK_HUART, hdmatx, hdma_link_tx);

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
#endif
}

#if LINK_OVER_STLINK
extern "C" void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_link_tx);
}
#endif

std::uint32_t UART_TX_Dropped(void) { return tx_dropped; }

int UART_TX_Send(const char* data, int len) {
    if (len <= 0 || len > UART_TX_LINE_SIZE) return 0;  // a bug

    TxLine* buffer = tx.assign();
    if (buffer == nullptr) {
        ++tx_dropped;
        return 0;
    }

    std::memcpy(buffer->data, data, static_cast<std::size_t>(len));
    buffer->len = static_cast<std::uint16_t>(len);
    tx.release();
    tx_start();

    return 1;
}

void UART_TX_OnError(void) {
    tx.operation_is_completed();   // the DMA stopped: the line is lost
    tx_start();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart != LINK_HUART) return;

    tx.operation_is_completed();
    tx_start();
}
