#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
bool run_all_tests(void);
uint8_t c_perform_circular_list_operations(uint32_t max_nodes);

void comparing_circular_lists(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Run tests
	if (!run_all_tests()) return 0;  // increases cold cycles run by ~2*max_nodes cycles

	// Useless Cold Measurement Isolation
	__asm volatile(".balign 16");	// 1. Flash alignment for the benchmark block
	__DSB();						// 2. Data Synchronization Barrier: Wait for all pending memory stores to finish
	__ISB();						// 3. Instruction Synchronization Barrier: Flush the CPU pipeline completely

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 10318 | warm cycles = 10268 | size = 476 bytes
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

	// Clang -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi:
	// cold cycles = 10061 | warm cycles = 10020 | size = 496 bytes
	start = DWT->CYCCNT;
	uint8_t clang_circular_list_status = clang_asm_perform_circular_list_operations(max_nodes);
	if (clang_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	clang_circular_list_status = clang_asm_perform_circular_list_operations(max_nodes);
	if (clang_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_circular_list_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;
}
