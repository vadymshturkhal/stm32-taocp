#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_queue.h"


extern bool asm_enqueue(Queue* queue, uint32_t info);
// AAPCS guarantees uint64_t returns in R0 (low) and R1 (high)
extern uint64_t asm_dequeue(Queue* queue);


uint8_t asm_test_queue_values(uint16_t max_nodes) {
	if (max_nodes == 0) return 0;

	void* mem = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (mem == NULL) return 0;

	Queue* queue = c_create_queue(mem, max_nodes);
	uint64_t raw_result;
	uint32_t success;
	uint32_t info;

	// Push phase
	for (int16_t i = max_nodes - 1; i > -1; i--) {
		if (asm_enqueue(queue, i) == false) {
			asm_balloc_free(mem);
			return 0;
		}
	}

	// Pop phase
	for (int16_t i = max_nodes - 1; i > -1; i--) {
		raw_result = asm_dequeue(queue); // R0 -> lower 32, R1 -> upper 32

		success = (uint32_t)(raw_result & 0xFFFFFFFF); // Extract R0
		info    = (uint32_t)(raw_result >> 32);        // Extract R1

		if (success == 0) {
			asm_balloc_free(mem);
			return 0; // Failed: Premature Underflow
		}

		if (info != (uint32_t)i) {
			asm_balloc_free(mem);
			return 0; // Failed: Data Corruption or LIFO behavior!
		}
	}

	asm_balloc_free(mem);
	return 1;
}

uint8_t asm_test_queue_boundaries(uint16_t max_nodes) {
	if (max_nodes == 0) return 0;

	void* mem = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (!mem) return 0;
	Queue* queue = c_create_queue(mem, max_nodes);

	uint64_t raw_result;
	uint32_t success;

	// TRAP 1: Instant Underflow (Pop from empty)
	raw_result = asm_dequeue(queue);
	success = (uint32_t)(raw_result & 0xFFFFFFFF); // Extract R0

	if (success != 0) goto fail; // It should have failed!

	// TRAP 2: Fill to absolute maximum
	for (uint16_t i = 0; i < max_nodes; i++) {
		if (asm_enqueue(queue, i) == false) goto fail;
	}

	// TRAP 3: Instant Overflow (Push past max)
	if (asm_enqueue(queue, 999) == true) goto fail; // It should have failed!

	asm_balloc_free(mem);
	return 1;

fail:
	asm_balloc_free(mem);
	return 0;
}

uint8_t asm_test_queue_caterpillar(uint16_t max_nodes) {
	if (max_nodes < 2) return 0; // Need at least 2 nodes

	void* mem = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (!mem) return 0;
	Queue* queue = c_create_queue(mem, max_nodes);

	uint64_t raw_result;
	uint32_t success;
	uint32_t info;

	// Crawl through memory. Forces Torvalds double-pointer to wrap/reset.
	for (uint16_t i = 0; i < max_nodes * 2; i++) {

		// Push one node
		if (asm_enqueue(queue, i) == false) goto fail;

		// Pop one node immediately
		raw_result = asm_dequeue(queue);
		success = (uint32_t)(raw_result & 0xFFFFFFFF); // Extract R0
		info    = (uint32_t)(raw_result >> 32);        // Extract R1

		if (success == 0 || info != (uint32_t)i) goto fail;
	}

	asm_balloc_free(mem);
	return 1;

fail:
	asm_balloc_free(mem);
	return 0;
}

uint8_t run_all_asm_queue_tests(uint16_t max_nodes) {
	if (!asm_test_queue_values(max_nodes)) return 0;
	if (!asm_test_queue_boundaries(max_nodes)) return 0;
	if (!asm_test_queue_caterpillar(max_nodes)) return 0;

	return 1; // Silicon approved!
}
