#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "storage_pool.h"
#include "doubly_linked_list.h"
#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

// Return 1 if storage_and_list_memory == NULL
// Return 2 if storage_pool == NULL
// Return 0 if OK
uint32_t test_doubly_linked_list(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 0;

	// max_nodes + 1 means pool size + head node
	uint32_t storage_pool_size = sizeof(Storage_Pool) + (max_nodes + 2) * sizeof(DoublyNode);
	uint32_t storage_and_list_size = storage_pool_size + sizeof(DoublyLinkedList);

	void* storage_and_list_memory = asm_balloc(storage_and_list_size);
	if (storage_and_list_memory == NULL) return 1;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_list_memory, sizeof(DoublyNode), max_nodes + 2);
	if (storage_pool == NULL) return 2;

	uint8_t* list_memory_start = (uint8_t*)storage_and_list_memory + storage_pool_size;

	DoublyLinkedList* list = (DoublyLinkedList*)(list_memory_start);

	doubly_linked_list_init(list, storage_pool);

	// Insert Left max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = doubly_linked_list_insert_left(list, i);
		if (status != 0) {
			asm_balloc_free(storage_and_list_memory);
			return status;
		}
	}

	// Pop Left max_nodes times
	uint32_t info;
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = doubly_linked_list_pop_left(list, &info);
		if (status != 0) {
			asm_balloc_free(storage_and_list_memory);
			return status;
		}
	}

	// Insert Right max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = doubly_linked_list_insert_right(list, i);
		if (status != 0) {
			asm_balloc_free(storage_and_list_memory);
			return status;
		}
	}

	// Clear
	doubly_linked_list_clear(list);

	// Insert Right max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = doubly_linked_list_insert_right(list, i);
		if (status != 0) {
			asm_balloc_free(storage_and_list_memory);
			return status;
		}
	}

	// Union
	volatile DoublyLinkedList stack_list;
	volatile DoublyLinkedList* another_list = &stack_list;
	doubly_linked_list_init(another_list, storage_pool);
	doubly_linked_list_union(another_list, list);

	// Pop Right max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = doubly_linked_list_pop_right(&info, another_list);
		if (status != 0) {
			asm_balloc_free(storage_and_list_memory);
			return status;
		}
	}

	asm_balloc_free(storage_and_list_memory);
	return 0;
}
