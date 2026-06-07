#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
uint8_t test_circular_list_head(uint32_t max_nodes);

uint8_t measure_circular_list_head_performance(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Translation Unit Boundary
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 13663 | warm cycles = 13544 | size = 328 bytes
	start = DWT->CYCCNT;
	uint8_t gcc_circular_list_head_status = test_circular_list_head(max_nodes);
	if (gcc_circular_list_head_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_head_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_circular_list_head_status = test_circular_list_head(max_nodes);
	if (gcc_circular_list_head_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_head_cycles_warm = (end - start) - overhead;




	end = DWT->CYCCNT;
}
