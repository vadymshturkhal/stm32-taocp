/* uart_tx.h - bytes sent on the link UART (LINK_HUART in main.h) using DMA. */
#ifndef UART_TX_H
#define UART_TX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_TX_LINE_SIZE 300

/* Call once after BSP_COM_Init() / MX_USART1_UART_Init(), before the first send.*/
void UART_TX_Init(void);

/* What UART_TX_Send() and UART_TX_Commit() return */
enum { UART_TX_OK = 0, UART_TX_BAD_LENGTH = 1, UART_TX_FULL = 2 };

/* Queue one line of len bytes (1...UART_TX_LINE_SIZE). Never waits. Returns UART_TX_OK,
 * UART_TX_BAD_LENGTH (a bug in the caller), or UART_TX_FULL (counted, see UART_TX_Dropped). */
int UART_TX_Send(const char *data, size_t len);

/* Lines refused because all buffers were busy, since start-up. */
uint32_t UART_TX_Dropped(void);

/* From HAL_UART_ErrorCallback on a DMA error: drop the line, keep sending. */
void UART_TX_OnError(void);

/* Create a line inside of DMA buffer. 
 * Gives the free buffer with UART_TX_LINE_SIZE or NULL. */
char* UART_TX_Begin(void);
int UART_TX_Commit(size_t len);

#ifdef __cplusplus
}
#endif

#endif /* UART_TX_H */
