#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
uint8_t c_perform_circular_list_operations(uint32_t max_nodes);
uint8_t c_perform_circular_list_operations_storage_pool(uint32_t max_nodes);

void comparing_circular_lists_abstraction_tax(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Useless Cold Measurement Isolation
	// __asm volatile(".balign 16");	// 1. Flash alignment for the benchmark block
	// __DSB();						// 2. Data Synchronization Barrier: Wait for all pending memory stores to finish
	// __ISB();						// 3. Instruction Synchronization Barrier: Flush the CPU pipeline completely

	// Storage Pool integrated
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 10318-10319 | warm cycles = 10268 | size = 476 bytes
	// GCC -O3 -mcpu=cortex-m4 -mthumb -flto: cold cycles = 6061 | warm cycles = 5983 | size = ? bytes
	start = DWT->CYCCNT;
	uint8_t gcc_circular_list_status = c_perform_circular_list_operations(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_circular_list_status = c_perform_circular_list_operations(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_warm = (end - start) - overhead;

	// Storage Pool independent
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 20343 | warm cycles = 20003 | size = 520 bytes
	// GCC -O3 -mcpu=cortex-m4 -mthumb -flto: cold cycles = 7294 | warm cycles = 7242 | size = ? bytes
	start = DWT->CYCCNT;
	uint8_t gcc_circular_list_storage_pool_status = c_perform_circular_list_operations_storage_pool(max_nodes);
	if (gcc_circular_list_storage_pool_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_storage_pool_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_circular_list_storage_pool_status = c_perform_circular_list_operations_storage_pool(max_nodes);
	if (gcc_circular_list_storage_pool_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_storage_pool_cycles_warm = (end - start) - overhead;




	end = DWT->CYCCNT;
}
