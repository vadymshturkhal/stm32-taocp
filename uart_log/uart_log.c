#include <stdbool.h>

#include "uart_log.h"

// Kept private to this module,
// stm32g4xx_it.c reaches it only through uart_log_dma_irq_handler() below
static DMA_HandleTypeDef hdma_lpuart1_tx;

// Stashed by uart_log_init() so async_printf() doesn't need to reach into
// BSP's hcom_uart[] global directly.
static UART_HandleTypeDef* uart_log_huart;

#define UART_LOG_BUF_SIZE 4096

// Ring buffer
static uint8_t  uart_log_buf[UART_LOG_BUF_SIZE];
static volatile uint16_t uart_log_head = 0;  // written only by the DMA completion callback
static volatile uint16_t uart_log_tail = 0;  // written only by async_printf()
static volatile bool     uart_log_busy = false;
static volatile uint16_t uart_log_inflight_len = 0;  // bytes in the transfer currently in flight

// Attaches a DMA channel to huart's TX side
// for hndling a whole buffer instead of blocking byte-by-byte in HAL_UART_Transmit()
// NOTE: huart must already be initialized with BSP_COM_Init()
void uart_log_init(UART_HandleTypeDef* huart)
{
    uart_log_huart = huart;

    // Peripheral clocks are gated off by default; DMAMUX1 is G4's request
    // router (Init.Request below), separate from the DMA1 controller itself.
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_lpuart1_tx.Instance                 = DMA1_Channel1;               // arbitrary free channel
    hdma_lpuart1_tx.Init.Request             = DMA_REQUEST_LPUART1_TX;      // DMAMUX: trigger on LPUART1 TX-empty
    hdma_lpuart1_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;        // RAM buffer -> UART data register
    hdma_lpuart1_tx.Init.PeriphInc           = DMA_PINC_DISABLE;            // always the same UART DR address
    hdma_lpuart1_tx.Init.MemInc              = DMA_MINC_ENABLE;             // next byte from buffer each transfer
    hdma_lpuart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_lpuart1_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_lpuart1_tx.Init.Mode                = DMA_NORMAL;                  // one-shot per buffer, not DMA_CIRCULAR
    hdma_lpuart1_tx.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_lpuart1_tx);     // Takes everything configured above and actually programs the DMA controller's hardware registers to match

    __HAL_LINKDMA(huart, hdmatx, hdma_lpuart1_tx);  // huart->hdmatx = &hdma_lpuart1_tx

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);  // without this the completion IRQ never reaches the CPU

    // DMA_NORMAL-mode TX completion is finished off by the UART peripheral's
    // OWN Transmission-Complete interrupt (see UART_DMATransmitCplt() in
    // stm32g4xx_hal_uart.c), not the DMA channel's interrupt -- without this,
    // HAL_UART_TxCpltCallback() never fires and the second message stays
    // stuck in the ring buffer forever.
    HAL_NVIC_SetPriority(LPUART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
}

// Called from DMA1_Channel1_IRQHandler (stm32g4xx_it.c). Clears the DMA
// status flags and, for a transfer started via HAL_UART_Transmit_DMA(),
// arms the UART's own TC interrupt (see uart_log_init()'s note above).
void uart_log_dma_irq_handler(void)
{
    HAL_DMA_IRQHandler(&hdma_lpuart1_tx);
}

// Called from LPUART1_IRQHandler (stm32g4xx_it.c)
// This triggers HAL_UART_TxCpltCallback() below
void uart_log_uart_irq_handler(void)
{
    HAL_UART_IRQHandler(uart_log_huart);
}

// Bytes currently queued (written but not yet transmitted), correctly
// handling the case where tail has wrapped past the buffer end while head
// hasn't caught up. NOTE: uint16_t operands promote to (signed) int in
// C's usual arithmetic conversions, so a bare "(tail - head) % SIZE" can
// go negative and give a wrong, huge result once reinterpreted as
// uint16_t -- the "+ UART_LOG_BUF_SIZE" keeps the dividend non-negative
// so the modulo is always correct, regardless of buffer size.
static uint16_t uart_log_used_space(void)
{
    return (uart_log_tail - uart_log_head + UART_LOG_BUF_SIZE) % UART_LOG_BUF_SIZE;
}

// Free space reserves one slot so a full buffer (tail caught up to head)
// is never indistinguishable from an empty one (both would otherwise read
// as tail == head).
static uint16_t uart_log_free_space(void)
{
    return (UART_LOG_BUF_SIZE - 1) - uart_log_used_space();
}

static void uart_log_dispatch_next(void)
{
    uint16_t pending = uart_log_used_space();
    if (pending == 0) {
        uart_log_busy = false;
        return;
    }
    uint16_t contiguous = UART_LOG_BUF_SIZE - uart_log_head;
    uint16_t chunk = (pending < contiguous) ? pending : contiguous;

    uart_log_inflight_len = chunk;
    HAL_UART_Transmit_DMA(uart_log_huart, &uart_log_buf[uart_log_head], chunk);
}

void async_printf(const uint8_t* buf, size_t len)
{
    // Overflow protection
    if (len > uart_log_free_space()) {
        return;
    }

    uint16_t tail = uart_log_tail;
    for (size_t i = 0; i < len; i++) {
        uart_log_buf[tail] = buf[i];
        tail = (tail + 1) % UART_LOG_BUF_SIZE;
    }
    uart_log_tail = tail;


    if (!uart_log_busy) {
        uart_log_busy = true;
        uart_log_dispatch_next();
    }
}

// sync
void uart_log_print_remaining(void)
{
    while (uart_log_busy) { }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    uart_log_head = (uart_log_head + uart_log_inflight_len) % UART_LOG_BUF_SIZE;
    uart_log_dispatch_next();
}
