#include "main.h"
#include "c_stack.h"

//#define MAX_NODES 128

// Prototypes
uint8_t perform_c_stack_operations_inline(uint16_t max_nodes);
uint8_t asm_perform_stack_operations_inline(uint16_t max_nodes);
uint8_t asm_perform_stack_operations_inline_hoisting(uint16_t max_nodes);
uint8_t asm_perform_stack_operations_inline_pingpong(uint16_t max_nodes);

void comparing_stack_inline() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;
    volatile uint8_t asm_stack_status;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3
	// with 128 nodes, 128 Push and 128 Pop using balloc (custom malloc)
	// cycles_cold = [4225-4231], cycles_warm = [4172-4181], size = 200 bytes
	start = DWT->CYCCNT;
	volatile uint8_t c_stack_status = perform_c_stack_operations_inline(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_stack_status = perform_c_stack_operations_inline(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// with 128 nodes, 128 Push and 128 Pop using balloc (custom malloc)
	// cycles_cold = [2606-2626], cycles_warm = [2554-2555], size = 390 bytes
	start = DWT->CYCCNT;
	asm_stack_status = asm_perform_stack_operations_inline_pingpong(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_stack_status = asm_perform_stack_operations_inline_pingpong(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_warm = (end - start) - overhead;




	end = DWT->CYCCNT;

}
