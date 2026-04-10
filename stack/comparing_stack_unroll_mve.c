#include "main.h"
#include "c_stack.h"

//#define MAX_NODES 128

// Prototypes
uint8_t perform_c_stack_operations_unroll(uint16_t max_nodes);
extern uint8_t asm_perform_stack_operations_inline_mve(uint16_t max_nodes);

void comparing_stack_unroll_mve() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;
    volatile uint8_t asm_stack_status;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3
	// Base case: 128 nodes, 128 Push/Pop using Bump Allocator (balloc)
	// cycles_cold = [3422-3423], cycles_warm = 3268, size = 448 bytes
	start = DWT->CYCCNT;
	volatile uint8_t c_stack_status = perform_c_stack_operations_unroll(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_stack_status = perform_c_stack_operations_unroll(max_nodes);
	if (c_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// Base case: 128 nodes, 128 Push/Pop using Bump Allocator (balloc)
	// cycles_cold = [2316-2344], cycles_warm = 2239, size = 372 bytes
	start = DWT->CYCCNT;
	asm_stack_status = asm_perform_stack_operations_inline_mve(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_stack_status = asm_perform_stack_operations_inline_mve(max_nodes);
	if (asm_stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_stack_cycles_warm = (end - start) - overhead;




	end = DWT->CYCCNT;

}
