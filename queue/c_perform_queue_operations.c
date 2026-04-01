#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_queue.h"


// Prototypes
bool c_enqueue_tub(Queue* queue, uint32_t info);
uint32_t c_dequeue_tub(Queue* queue, bool* pop_is_success);


uint8_t c_perform_queue_operations(uint16_t max_nodes) {
	// node info is uint32_t

	if (max_nodes == 0) return 0;

	void* c_queue_memory = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
	if (c_queue_memory == NULL) return 0;

	Queue* queue = c_create_queue(c_queue_memory, max_nodes);
	bool pop_is_success = true;	// flag for Underflow checking
	uint32_t info;

	for (uint32_t i = max_nodes; i > 0; i--){
		if (c_enqueue_tub(queue, i) == false) {
			asm_balloc_free(c_queue_memory);
			return 0;
		}
	}

	for (uint32_t i = max_nodes; i > 0; i--){
		info = c_dequeue_tub(queue, &pop_is_success);
		if (pop_is_success == false) {
			asm_balloc_free(c_queue_memory);
			return 0;
		}
	}

	asm_balloc_free(c_queue_memory);
	return 1;
}
