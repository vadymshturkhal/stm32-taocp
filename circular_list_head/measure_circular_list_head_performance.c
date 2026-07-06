#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
uint8_t test_circular_list_head(uint32_t max_nodes);

uint8_t measure_circular_list_head_performance() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Translation Unit Boundary
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 14307 | warm cycles = 14187 |
	// Flash size = 352 bytes: test_circular_list_head(0x70) + balloc(0xc + 0x28) + storage_pool(0x38 + 0x14 + 0x18) + circular_list_head(0x8 + 0x28 + 0x28)
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

	return 0;
}
