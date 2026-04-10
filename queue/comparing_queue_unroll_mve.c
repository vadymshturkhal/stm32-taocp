#include "main.h"


// Prototypes
uint8_t run_all_queue_tests(uint16_t max_nodes);
uint8_t run_all_asm_queue_tests(uint16_t max_nodes);

uint8_t c_perform_queue_operations_unroll(uint16_t max_nodes);
uint8_t c_perform_queue_operations_integrated(uint16_t max_nodes);
extern uint8_t asm_perform_queue_operations_mve4(uint16_t max_nodes);


void comparing_queue_unroll_mve() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// GCC -O3 (C)
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [2747-2835], cycles_warm = 2653-2678, size = 428 bytes
	start = DWT->CYCCNT;
	volatile uint8_t c_queue_status = c_perform_queue_operations_integrated(max_nodes);
	if (c_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_queue_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_queue_status = c_perform_queue_operations_integrated(max_nodes);
	if (c_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_queue_cycles_warm = (end - start) - overhead;


	// ARM Assembly
	// with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
	// cycles_cold = [2210-2237], cycles_warm = 2120, size = 360 bytes
	start = DWT->CYCCNT;
	volatile uint8_t asm_queue_status = asm_perform_queue_operations_mve4(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t rust_asm_queue_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_queue_status = asm_perform_queue_operations_mve4(max_nodes);
	if (asm_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t rust_asm_queue_cycles_warm = (end - start) - overhead;








	end = DWT->CYCCNT;

}
