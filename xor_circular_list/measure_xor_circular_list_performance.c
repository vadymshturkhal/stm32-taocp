#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
uint32_t test_xor_circular_list(uint32_t max_nodes);

__attribute__((noinline, optimize("O0")))
uint32_t measure_xor_circular_list_performance() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3 -mcpu=cortex-m4 -mthumb (Translation Unit Boundary): cold cycles = 17613 | warm cycles = 17528
	// Flash size = 444 bytes: test_xor_circular_list(0x84) + balloc(0xc + 0x28) + storage_pool(0x38 + 0x14 + 0x18) + xor_circular_list(0xc + 0x48 + 0x4c)
	// GCC -O3 -mcpu=cortex-m4 -mthumb -flto: cold cycles = 9331 | warm cycles = 9194
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
