#include "main.h"


// Prototypes
uint8_t run_all_queue_tests(uint16_t max_nodes);
uint8_t run_all_asm_queue_tests(uint16_t max_nodes);

uint8_t c_perform_queue_operations_unroll(uint16_t max_nodes);
uint8_t asm_perform_queue_operations_mve4(uint16_t max_nodes);


void comparing_queue_unroll_mve() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Run tests
//	uint8_t c_queue_test_status = run_all_queue_tests(max_nodes);
//	if (c_queue_test_status == 0) return;
//	uint8_t asm_queue_test_status = run_all_asm_queue_tests(max_nodes);
//	if (asm_queue_test_status == 0) return;

	// GCC -O3
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [3724], cycles_warm = [3653], size = 376 bytes
//	start = DWT->CYCCNT;
//	volatile uint8_t c_queue_status = c_perform_queue_operations_unroll(max_nodes);
//	if (c_queue_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t c_queue_cycles_cold = (end - start) - overhead;
//
//	start = DWT->CYCCNT;
//	c_queue_status = c_perform_queue_operations_unroll(max_nodes);
//	if (c_queue_status == 0) return;
//	end = DWT->CYCCNT;
//	volatile uint32_t c_queue_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [2431-2456], cycles_warm = [2373], size = 372 bytes
	// cycles_cold = [2327], cycles_warm = [2246], size = ? bytes
	// cycles_cold = [2198], cycles_warm = [2118], size = ? bytes
	start = DWT->CYCCNT;
	volatile uint8_t asm_queue_status = asm_perform_queue_operations_mve4(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_queue_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_queue_status = asm_perform_queue_operations_mve4(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t asm_queue_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;

}
