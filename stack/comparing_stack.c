#include "main.h"
#include "c_stack.h"


// Prototypes
uint8_t perform_c_stack_operations(uint16_t max_nodes);
uint8_t asm_perform_stack_operations(uint16_t max_nodes);


void comparing_stack() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3
	// Translation Unit Boundary Push/Pop case (not integrated)
	// with 128 nodes, 128 Push and 128 Pop using Bump Allocator (balloc)
	// cycles_cold = 9119, cycles_warm = 9051, size = 268 bytes
	start = DWT->CYCCNT;
	volatile uint8_t c_stack_status = perform_c_stack_operations(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_stack_status = perform_c_stack_operations(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// Translation Unit Boundary Push/Pop case (not integrated)
	// with 128 nodes, 128 Push and 128 Pop using Bump Allocator (balloc)
	// cycles_cold = [5988-6138], cycles_warm = 6083, size = 212 bytes
	start = DWT->CYCCNT;
	volatile uint8_t asm_stack_status = asm_perform_stack_operations(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_stack_status = asm_perform_stack_operations(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;

}
