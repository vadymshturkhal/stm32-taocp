#ifndef UART_LOG_H
#define UART_LOG_H

#include <stdint.h>
#include <stddef.h>
#include "stm32g4xx_hal.h"

#include "uart_log_list.h"

// DMA-driven UART logging: replaces the blocking, byte-by-byte HAL_UART_Transmit()
// used by __io_putchar with a single non-blocking HAL_UART_Transmit_DMA() call per
// write, freeing the CPU from busy-waiting during trace/printf output.

// Sets up the LPUART1 TX DMA channel and links it to huart. Call once, after
// BSP_COM_Init() has initialized huart.
void uart_log_init(UART_HandleTypeDef* huart);

// Forwards to HAL_DMA_IRQHandler(); call from DMA1_Channel1_IRQHandler().
void uart_log_dma_irq_handler(void);

// Forwards to HAL_UART_IRQHandler(); call from LPUART1_IRQHandler().
void uart_log_uart_irq_handler(void);

// Queues len bytes from buf for transmission, dispatching immediately via DMA
// if the UART is idle.
void async_printf(const uint8_t* buf, size_t len);

// Blocks until everything currently queued has genuinely finished
// transmitting. Only needed when you require certainty (before a
// reset/power-down/end of a test run) -- normal logging stays async.
void uart_log_print_remaining(void);

#endif
