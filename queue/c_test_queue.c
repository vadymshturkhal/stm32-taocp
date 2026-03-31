#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_queue.h"


// Prototypes
bool c_enqueue_tub(Queue* queue, uint32_t info);
uint32_t c_dequeue_tub(Queue* queue, bool* pop_is_success);


uint8_t c_test_queue_values(uint16_t max_nodes) {
	// node info is uint32_t

	void* c_queue_memory = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (c_queue_memory == NULL) return 0;

	Queue* queue = c_create_queue(c_queue_memory, max_nodes);
	bool pop_is_success = true;	// flag for Underflow checking
	uint32_t info;

	for (int16_t i = max_nodes - 1; i > -1; i--){
		if (c_enqueue_tub(queue, i) == false) {
			asm_balloc_free(c_queue_memory);
			return 0;
		}
	}

	for (int16_t i = max_nodes - 1; i > -1; i--){
		info = c_dequeue_tub(queue, &pop_is_success);
		if (pop_is_success == false) {
			asm_balloc_free(c_queue_memory);
			return 0; // Failed: Premature Underflow
		}

		// THE TRUTH CHECK
		if (info != (uint32_t)i) {
			asm_balloc_free(c_queue_memory);
			return 0; // Failed: Data Corruption or LIFO behavior!
		}
	}

	asm_balloc_free(c_queue_memory);
	return 1;
}

uint8_t c_test_queue_boundaries(uint16_t max_nodes) {
	void* mem = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (!mem) return 0;
	Queue* queue = c_create_queue(mem, max_nodes);
	bool success;

	// TRAP 1: Instant Underflow (Pop from empty)
	c_dequeue_tub(queue, &success);
	if (success == true) goto fail; // It should have failed!

	// TRAP 2: Fill to absolute maximum
	for (uint16_t i = 0; i < max_nodes; i++) {
		if (c_enqueue_tub(queue, i) == false) goto fail;
	}

	// TRAP 3: Instant Overflow (Push past max)
	if (c_enqueue_tub(queue, 999) == true) goto fail; // It should have failed!

	asm_balloc_free(mem);
	return 1;

fail:
	asm_balloc_free(mem);
	return 0;
}

uint8_t c_test_queue_caterpillar(uint16_t max_nodes) {
	if (max_nodes < 2) return 0; // Need at least 2 nodes

	void* mem = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (!mem) return 0;
	Queue* queue = c_create_queue(mem, max_nodes);
	bool success = true;
	uint32_t info;

	// Crawl through memory by pushing and popping repeatedly.
	// This forces the Torvalds double-pointer to constantly reset.
	for (uint16_t i = 0; i < max_nodes * 2; i++) {
		// Push one node
		if (c_enqueue_tub(queue, i) == false) goto fail;

		// Pop one node immediately (Queue becomes empty again)
		info = c_dequeue_tub(queue, &success);
		if (!success || info != i) goto fail;
	}

	asm_balloc_free(mem);
	return 1;

fail:
	asm_balloc_free(mem);
	return 0;
}

uint8_t run_all_queue_tests(uint16_t max_nodes) {
    if (!c_test_queue_values(max_nodes)) return 0; // Fails data integrity
    if (!c_test_queue_boundaries(max_nodes)) return 0;    // Fails fail-doors
    if (!c_test_queue_caterpillar(max_nodes)) return 0;   // Fails Torvalds math

    return 1; // All silicon approved!
}
