#include "main.h"


// Prototypes
uint8_t run_all_queue_tests(uint16_t max_nodes);
uint8_t c_perform_queue_operations(uint16_t max_nodes);


void comparing_queue() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;
	uint8_t queue_test_status = run_all_queue_tests(max_nodes);
	if (queue_test_status == 0) return;

	// GCC -O3
	// Translation Unit Boundary Push/Pop case (not integrated)
	// with 128 nodes, 128 Push and 128 Pop using Bump Allocator (balloc)
	// cycles_cold = [9468-9507], cycles_warm = 9438, size = 288 bytes
	start = DWT->CYCCNT;
	volatile uint8_t c_queue_status = c_perform_queue_operations(max_nodes);
	if (c_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_queue_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_queue_status = c_perform_queue_operations(max_nodes);
	if (c_queue_status == 0) return;
	end = DWT->CYCCNT;
	volatile uint32_t c_queue_cycles_warm = (end - start) - overhead;




	end = DWT->CYCCNT;

}
