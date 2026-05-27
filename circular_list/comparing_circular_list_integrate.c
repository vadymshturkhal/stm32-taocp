#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
bool run_all_tests(void);
uint8_t c_perform_circular_list_operations_integrate(uint32_t max_nodes);
extern uint8_t clang_asm_perform_circular_list_operations_integrate(uint32_t max_nodes);
extern uint8_t asm_perform_circular_list_operations_integrate(uint32_t max_nodes);

void comparing_circular_lists_integrate(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// Useless Cold Measurement Isolation
	// __asm volatile(".balign 16");	// 1. Flash alignment for the benchmark block
	// __DSB();						// 2. Data Synchronization Barrier: Wait for all pending memory stores to finish
	// __ISB();						// 3. Instruction Synchronization Barrier: Flush the CPU pipeline completely

	const uint16_t max_nodes = 128;

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 5821 | warm cycles = 5770 | size = 340 bytes
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 3946 | warm cycles = 3886 | size = 396 bytes
	start = DWT->CYCCNT;
	uint8_t gcc_circular_list_status = c_perform_circular_list_operations_integrate(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_circular_list_status = c_perform_circular_list_operations_integrate(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_warm = (end - start) - overhead;

	// Clang -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi:
	// cold cycles = 3410 | warm cycles = 3313 | size = 702 bytes
	start = DWT->CYCCNT;
	uint8_t clang_circular_list_status = clang_asm_perform_circular_list_operations_integrate(max_nodes);
	if (clang_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	clang_circular_list_status = clang_asm_perform_circular_list_operations_integrate(max_nodes);
	if (clang_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_circular_list_cycles_warm = (end - start) - overhead;

	// ASM: cold cycles = 4898 | warm cycles = 4827 | size = 366 bytes
	// ASM: cold cycles = 4771 | warm cycles = 4771 | size = 370 bytes
	// ASM: cold cycles = 4646 | warm cycles = 4580 | size = 374 bytes
	// ASM: cold cycles = 4678 | warm cycles = 4608 | size = 398 bytes
	// ASM: cold cycles = 4493 | warm cycles = 4419 | size = 418 bytes
	// ASM: cold cycles = 4470 | warm cycles = 4392 | size = 410 bytes
	// ASM: cold cycles = 4433 | warm cycles = 4355 | size = 410 bytes
	// ASM: cold cycles = 4371 | warm cycles = 4289 | size = 418 bytes
	// ASM: cold cycles = 4247 | warm cycles = 4163 | size = 442 bytes
	// ASM: cold cycles = 4187 | warm cycles = 4099 | size = 442 bytes
	// ASM: cold cycles = 4124 | warm cycles = 4037 | size = 442 bytes
	// ASM: cold cycles = 3769 | warm cycles = 3682 | size = 446 bytes
	// ASM: cold cycles = 3708 | warm cycles = 3617 | size = ? bytes
	// ASM: cold cycles = 3581 | warm cycles = 3492 | size = ? bytes
	// ASM: cold cycles = 3461 | warm cycles = 3366 | size = 470 bytes
	// ASM: cold cycles = 3398 | warm cycles = 3302 | size = 470 bytes
	// ASM: cold cycles = 3335 | warm cycles = 3238 | size = 470 bytes
	// ASM: cold cycles = 2928 | warm cycles = 2830 | size = ? bytes
	// ASM: cold cycles = 2865 | warm cycles = 2766 | size = 516 bytes
	start = DWT->CYCCNT;
	uint8_t asm_circular_list_status = asm_perform_circular_list_operations_integrate(max_nodes);
	if (asm_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_circular_list_status = asm_perform_circular_list_operations_integrate(max_nodes);
	if (asm_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_circular_list_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;
}
