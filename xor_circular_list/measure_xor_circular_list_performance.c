#include <stdint.h>
#include <stdbool.h>
#include "main.h"


// Prototypes
uint32_t test_xor_circular_list(uint32_t max_nodes);


uint32_t measure_xor_circular_list_performance() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Translation Unit Boundary
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 18037 | warm cycles = 17897 | size = 328 bytes
	start = DWT->CYCCNT;
	uint8_t gcc_xor_circular_list_status = test_xor_circular_list(max_nodes);
	if (gcc_xor_circular_list_status != 0) return gcc_xor_circular_list_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_xor_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_xor_circular_list_status = test_xor_circular_list(max_nodes);
	if (gcc_xor_circular_list_status != 0) return gcc_xor_circular_list_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_xor_circular_list_cycles_warm = (end - start) - overhead;

	return 0;
}
