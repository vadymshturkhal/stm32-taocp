/* uart_rx.h - bytes received on LPUART1 (BSP COM1), one interrupt per byte */
#ifndef UART_RX_H
#define UART_RX_H

#include <stdint.h>

#define UART_RX_BUF_SIZE 512        /* ring buffer: holds UART_RX_BUF_SIZE - 1 bytes */

/* Call once after BSP_COM_Init(). Enables the LPUART1 interrupt and starts receiving. */
void UART_RX_Init(void);

/* Take the next received byte. Returns 1 if *byte was set, 0 if nothing is waiting. */
int UART_RX_Read(uint8_t *byte);

/* Bytes lost so far: the buffer was full, or the UART overran. */
uint32_t UART_RX_Dropped(void);

#endif /* UART_RX_H */
