/* uart_tx.h - bytes sent on the link UART (LINK_HUART in main.h) using DMA. */
#ifndef UART_TX_H
#define UART_TX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_TX_LINE_SIZE 300

/* Call once after BSP_COM_Init() / MX_USART1_UART_Init(), before the first send.*/
void UART_TX_Init(void);

/* Queue one line of len bytes (1...UART_TX_LINE_SIZE). Never waits:
 * returns 1 if queued, 0 if all buffers are busy (counted, see UART_TX_Dropped). */
int UART_TX_Send(const char *data, int len);

/* Lines refused because all buffers were busy, since start-up. */
uint32_t UART_TX_Dropped(void);

/* From HAL_UART_ErrorCallback on a DMA error: drop the line, keep sending. */
void UART_TX_OnError(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_TX_H */
