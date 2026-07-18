#include <stdint.h>
#include <stdbool.h>

#include "main.h"

// Prototypes
uint32_t test_doubly_linked_list(uint32_t max_nodes);

__attribute__((noinline, optimize("O0")))
uint8_t measure_doubly_linked_list_performance() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3 -mcpu=cortex-m4 -mthumb (Translation Unit Boundary) for 128*6 operations: cold cycles = 39455-39456 | warm cycles = 39251
	// Flash size = ? bytes: balloc(0xc + 0x28) + storage_pool(0x38 + 0x14 + 0x18) + ...
	// GCC -O3 -mcpu=cortex-m4 -mthumb -flto for 128*5 operations: cold cycles = 16012 | warm cycles = 15804
	start = DWT->CYCCNT;
	uint8_t gcc_doubly_linked_list_status = test_doubly_linked_list(max_nodes);
	if (gcc_doubly_linked_list_status != 0) return gcc_doubly_linked_list_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_doubly_linked_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_doubly_linked_list_status = test_doubly_linked_list(max_nodes);
	if (gcc_doubly_linked_list_status != 0) return gcc_doubly_linked_list_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_doubly_linked_list_cycles_warm = (end - start) - overhead;

	return 0;
}
