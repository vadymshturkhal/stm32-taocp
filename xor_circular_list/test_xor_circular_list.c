#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "xor_circular_list.h"

#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);


// Return 1 if max_nodes == 0
// Return 2 if memory == NULL
// Return 3 if insert left error
// Return 4 if pop left error
// Else Return 0
uint32_t test_xor_circular_list(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 1;

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(XORCircularNode);
	uint32_t storage_and_circular_list_size = storage_pool_size + sizeof(XORCircularList);

	void* storage_and_circular_list_memory = asm_balloc(storage_and_circular_list_size);
	if (storage_and_circular_list_memory == NULL) return 2;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_circular_list_memory, sizeof(XORCircularNode), max_nodes);
	if (storage_pool == NULL) return 2;

	uint8_t* circular_list_memory_start = (uint8_t*)storage_and_circular_list_memory + storage_pool_size;

	XORCircularList* circular_list = (XORCircularList*)(circular_list_memory_start);

	xor_circular_list_init(circular_list, storage_pool);
	uint32_t info;

	// Insert Left max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
//		if (xor_circular_list_insert_left(circular_list, i) == false) {
		if (xor_circular_list_insert_right(circular_list, i) == false) {
			asm_balloc_free(storage_and_circular_list_memory);
			return 3;
		}
	}

//	XORCircularNodeHead* head1 = (XORCircularNodeHead*)circular_list;
//	XORCircularNodeHead* head2 = head1 + 1;
//
//	XORCircularNode* forward_node = (XORCircularNode*)(head2->link ^ (uintptr_t)head1);
//	XORCircularNode* forward_node1 = (XORCircularNode*)(forward_node->link ^ (uintptr_t)head2);
//	XORCircularNode* forward_node2 = (XORCircularNode*)(forward_node1->link ^ (uintptr_t)forward_node);
//	XORCircularNode* forward_node3 = (XORCircularNode*)(forward_node2->link ^ (uintptr_t)forward_node1);
//	XORCircularNode* forward_node4 = (XORCircularNode*)(forward_node3->link ^ (uintptr_t)forward_node2);
//	XORCircularNode* forward_node5 = (XORCircularNode*)(forward_node4->link ^ (uintptr_t)forward_node3);
//
//	XORCircularNode* backward_node = (XORCircularNode*)(head1->link ^ (uintptr_t)head2);
//	XORCircularNode* backward_node1 = (XORCircularNode*)(backward_node->link ^ (uintptr_t)head1);
//	XORCircularNode* backward_node2 = (XORCircularNode*)(backward_node1->link ^ (uintptr_t)backward_node);
//	XORCircularNode* backward_node3 = (XORCircularNode*)(backward_node2->link ^ (uintptr_t)backward_node1);

	// Pop max_nodes times
	for (uint32_t i = max_nodes; i > 0; i--){
		uint32_t status = xor_circular_list_pop_left(circular_list, &info);
		if (status != 0) {
			asm_balloc_free(storage_and_circular_list_memory);
			return status;
		}
	}

	asm_balloc_free(storage_and_circular_list_memory);
	return 0;
}
