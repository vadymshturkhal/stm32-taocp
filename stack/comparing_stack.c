#include "main.h"


#define MAX_NODES 128


// Prototypes
uint8_t perform_c_stack_operations(uint16_t max_nodes, uint8_t operations_factor);


void comparing_stack() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// perform push/pop (MAX_NODES * operations_factor) times
	uint8_t operations_factor = 1;

	// GCC -O3
	// with	nodes = 128, operations_factor = 1 (128 push and 128 pop)
	// cycles_cold = [4984-5124], cycles_warm = [4586-4700], size = 296 bytes
	start = DWT->CYCCNT;
	volatile uint8_t stack_status = perform_c_stack_operations(MAX_NODES, operations_factor);
	if (stack_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_stack_cycles_cold = (end - start) - overhead;
//
//	start = DWT->CYCCNT;
//	stack_status = perform_c_stack_operations(MAX_NODES, operations_factor);
//	if (stack_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t c_stack_cycles_warm = (end - start) - overhead;



	// ARM Assembly
	// with ?
	// cycles_cold = ?, cycles_warm = ?, size = ? bytes;
//	start = DWT->CYCCNT;
//	volatile uint8_t stack_status = perform_c_stack_operations(MAX_NODES, operations_factor);
//	if (stack_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t asm_stack_cycles_cold = (end - start) - overhead;


	end = DWT->CYCCNT;

}
