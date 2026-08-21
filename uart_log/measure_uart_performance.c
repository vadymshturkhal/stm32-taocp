#include <stdint.h>
#include <stddef.h>

#include "main.h"
#include "uart_log.h"

void async_printf(const uint8_t* buf, size_t len);
void uart_log_print_remaining(void);

uint32_t measure_uart_performance() {
    volatile uint32_t start, end, overhead;

    start = DWT->CYCCNT;
    end = DWT->CYCCNT;
    overhead = end - start;

    uart_log_init(&hcom_uart[COM1]);

    // Mirrors a real trace() line's shape/length so the measurement reflects
    // actual elevator-trace usage rather than a trivial short string.
    static const uint8_t mock_input[] = "0 N 1 0 0 0 E1 wait for call\r\n";
    const size_t mock_len = sizeof(mock_input) - 1;  // exclude NUL terminator
//    async_printf(mock_input, mock_len);

    // Second write, distinct text so it's obvious on the terminal whether
    // it came out or not.
    static const uint8_t mock_input2[] = "1 N 1 0 0 0 E2 second write\r\n";
    const size_t mock_len2 = sizeof(mock_input2) - 1;
    async_printf(mock_input, mock_len);
    async_printf(mock_input2, mock_len2);

    uart_log_print_remaining();

    // NOTE: the IRQ fires and reaches HAL_DMA_IRQHandler() fine, but
    // HAL_UART_TxCpltCallback() isn't defined yet (step 4) -- HAL's weak
    // no-op runs instead of draining the ring buffer, so uart_log_busy
    // never clears after the first dispatch.
    // start = DWT->CYCCNT;
    // async_printf(mock_input, mock_len);
    // end = DWT->CYCCNT;
    // volatile uint32_t async_printf_cycles_cold = (end - start) - overhead;

    // start = DWT->CYCCNT;
    // async_printf(mock_input, mock_len);
    // end = DWT->CYCCNT;
    // volatile uint32_t async_printf_cycles_warm = (end - start) - overhead;

    return 0;
}
