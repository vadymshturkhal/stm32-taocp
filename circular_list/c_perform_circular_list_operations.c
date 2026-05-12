#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_circular_list.h"
#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint8_t c_perform_circular_list_operations(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 0;

	void* c_circular_list_memory = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList));
	if (c_circular_list_memory == NULL) return 0;

	CircularList* circular_list = c_create_circular_list(c_circular_list_memory, max_nodes);
	uint32_t info;

	// Insert Left max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list_insert_left(circular_list, i) == false) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}

	// Pop max_nodes times
	bool pop_is_success = true;		// flag for Underflow checking
	for (uint32_t i = 128; i > 0; i--){
		info = circular_list_pop(&pop_is_success, circular_list);
		if (pop_is_success == false) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}

	// Insert Right max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list_insert_right(circular_list, i) == false)
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}

	circular_list_clear(circular_list);

	asm_balloc_free(c_circular_list_memory);
	return 1;
}
