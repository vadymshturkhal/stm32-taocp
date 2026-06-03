#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_circular_list_storage_pool.h"
#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint8_t c_perform_circular_list_operations_storage_pool(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 0;

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(CircularNode);
	uint32_t storage_and_circular_list_size = storage_pool_size + sizeof(CircularListStorage);

	void* storage_and_circular_list_memory = asm_balloc(storage_and_circular_list_size);
	if (storage_and_circular_list_memory == NULL) return 0;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_circular_list_memory, sizeof(CircularNode), max_nodes);

	uint8_t* circular_list_memory_start = (uint8_t*)storage_and_circular_list_memory + storage_pool_size;

	CircularListStorage* circular_list = (CircularListStorage*)(circular_list_memory_start);

	storage_init_circular_list(circular_list, storage_pool);
	uint32_t info;

	// Insert Left max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		if (storage_circular_list_insert_left(circular_list, i) == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 0;
		}
	}

	// Pop max_nodes times
	bool pop_is_success = true;		// flag for Underflow checking
	for (uint32_t i = max_nodes; i > 0; i--){
		info = storage_circular_list_pop(&pop_is_success, circular_list);
		if (pop_is_success == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 0;
		}
	}

	// Insert Right max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		if (storage_circular_list_insert_right(circular_list, i) == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 0;
		}
	}

	storage_circular_list_clear(circular_list);

	asm_balloc_free(storage_and_circular_list_memory);
	return 1;
}
