#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "storage_pool.h"

Storage_Pool* create_storage_pool(void* memory, uint32_t node_size, uint32_t storage_size) {
	// Every Node must begin with a link pointer as its first field

	if (node_size == 0 || storage_size == 0) return NULL;

	Storage_Pool* storage_pool = (Storage_Pool*)memory;
	storage_pool->node_size = node_size;
	storage_pool->avail = NULL;
	storage_pool->size = storage_size;

	// Nodes starting memory address
	uint8_t* memory_cursor = (uint8_t*)(storage_pool + 1);

	void* temp_avail = NULL;  // caching avail

	for (uint32_t i = storage_size; i > 0; i--) {
		void* current_node = memory_cursor;  // Memory address of the current node
		memory_cursor += node_size;		  // Memory address of the next node

		// Cast the node address to a void** so we can write to its link field
		void** link_field = (void**)current_node;

		// Link it to the previous avail node
		*link_field = temp_avail;
		temp_avail = current_node;
	}

	// Shift avail to this node
	storage_pool->avail = temp_avail;

	return storage_pool;
}

void* storage_pool_pop(Storage_Pool* storage_pool) {
	if (storage_pool->avail == NULL) {
		return NULL;  // Overflow
	}

	void* P = storage_pool->avail;
	void** P_link = (void**)P;
	storage_pool->avail = *P_link;
	storage_pool->size -= 1;

	return P;
}

bool storage_pool_push(Storage_Pool* storage_pool, void* node) {
	if (node == NULL) return false;

	void** link_field = (void**)node;
	*link_field = storage_pool->avail;
	storage_pool->avail = node;
	storage_pool->size += 1;

	return true;
}

// Insert a chain of Nodes
bool storage_pool_add_slice(Storage_Pool* storage_pool, void* head, void* tail) {
	if (storage_pool == NULL || head == NULL || tail == NULL) return false;

	void* P = storage_pool->avail;
	storage_pool->avail = head;

	void** tail_link = (void**)tail;
	*tail_link = P;

	return true;
}
