#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "circular_list_head.h"

#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
void circular_list_head_init(CircularListHead* circular_list, Storage_Pool* storage_pool);


uint8_t test_circular_list_head(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 0;

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(CircularNode);
	uint32_t storage_and_circular_list_size = storage_pool_size + sizeof(CircularListHead);

	void* storage_and_circular_list_memory = asm_balloc(storage_and_circular_list_size);
	if (storage_and_circular_list_memory == NULL) return 0;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_circular_list_memory, sizeof(CircularNode), max_nodes);
	if (storage_pool == NULL) return 0;

	uint8_t* circular_list_memory_start = (uint8_t*)storage_and_circular_list_memory + storage_pool_size;

	CircularListHead* circular_list = (CircularListHead*)(circular_list_memory_start);

	circular_list_head_init(circular_list, storage_pool);
	uint32_t info;

	// Insert Left max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list_head_insert_left(circular_list, i) == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 0;
		}
	}

	// Pop max_nodes times
	bool pop_is_success = true;		// flag for Underflow checking
	for (uint32_t i = max_nodes; i > 0; i--){
		info = circular_list_head_pop(&pop_is_success, circular_list);
		if (pop_is_success == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 0;
		}
	}

	asm_balloc_free(storage_and_circular_list_memory);
	return 1;
}
