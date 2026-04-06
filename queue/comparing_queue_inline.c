#include "main.h"


// Prototypes
uint8_t run_all_queue_tests(uint16_t max_nodes);
uint8_t run_all_asm_queue_tests(uint16_t max_nodes);

uint8_t c_perform_queue_operations_inline(uint16_t max_nodes);
uint8_t asm_perform_queue_operations_inline(uint16_t max_nodes);


void comparing_queue_inline() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Run tests
	uint8_t c_queue_test_status = run_all_queue_tests(max_nodes);
	if (c_queue_test_status == 0) return;
	uint8_t asm_queue_test_status = run_all_asm_queue_tests(max_nodes);
	if (asm_queue_test_status == 0) return;

	// GCC -O3
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [3957], cycles_warm = [3937], size = ? bytes
//	start = DWT->CYCCNT;
//	volatile uint8_t c_queue_status = c_perform_queue_operations_inline(max_nodes);
//	if (c_queue_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t c_queue_cycles_cold = (end - start) - overhead;
//
//	start = DWT->CYCCNT;
//	c_queue_status = c_perform_queue_operations_inline(max_nodes);
//	if (c_queue_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t c_queue_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [3179], cycles_warm = [3151], size = ? bytes
	start = DWT->CYCCNT;
	volatile uint8_t asm_queue_status = asm_perform_queue_operations_inline(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_queue_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_queue_status = asm_perform_queue_operations_inline(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_queue_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;

}
