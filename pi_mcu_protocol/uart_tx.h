/* uart_tx.h - bytes sent on LPUART1 (BSP COM1) using DMA */
#ifndef UART_TX_H
#define UART_TX_H

#define TX_BUF_SIZE 512

/* Call once after BSP_COM_Init() and before the first UART_TX_Send(). */
void UART_TX_Init(void);

/* Queue len bytes for sending. Waits only if the transmit buffer is full. */
void UART_TX_Send(const char *data, int len);

#endif /* UART_TX_H */
