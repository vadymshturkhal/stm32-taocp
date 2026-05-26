#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_circular_list.h"
#include "main.h"

// Using Topological Slice

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint8_t c_perform_circular_list_operations_integrate(uint32_t max_nodes) {
	// node info is uint32_t
	if (max_nodes == 0) return 0;

	void* c_circular_list_memory = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList));
	if (c_circular_list_memory == NULL) return 0;

	CircularList* circular_list = c_create_circular_list(c_circular_list_memory, max_nodes);
	uint32_t info;


// Insert Left max_nodes times
	// Same as Insert Right but started from the last
	CircularNode* P1 = circular_list->avail;
	CircularNode* P1_next = P1;

	for (uint32_t i = 1; i < max_nodes + 1; i++){
		P1 = P1_next;
		if (P1 == NULL) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}

		P1->info = i;
		P1_next = P1->link;
	}

	if (circular_list->ptr == NULL) {
		// List is currently empty, P points to itself
		P1->link = circular_list->avail;	// last point to first
		circular_list->ptr = P1;
		circular_list->avail = P1_next;
	} else {
		// Insert P at the front
		P1->link = circular_list->ptr->link;
		circular_list->ptr->link = circular_list->avail;
		circular_list->avail = P1_next;
	}

//	for (uint32_t i = max_nodes; i > 0; i--){
//		if (circular_list_insert_left_inline(circular_list, i) == false) {
//			asm_balloc_free(c_circular_list_memory);
//			return 0;
//		}
//	}

// Pop max_nodes times
	// Topological Slice
	if (circular_list->ptr == NULL || max_nodes == 0) {
		asm_balloc_free(c_circular_list_memory);
		return 0;
	}

	CircularNode* head = circular_list->ptr->link;
	CircularNode* P = head;

	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list->ptr == NULL) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}

		P = circular_list->ptr->link;

	    if (circular_list->ptr == P) {
	        circular_list->ptr = NULL;
	    } else {
			// FIXME: cache
	        circular_list->ptr->link = P->link;
	    }

		// P->link = circular_list->avail;
		// circular_list->avail = P;

		info = P->info;
	}

	// AVAIL <= P(slice)
	P->link = circular_list->avail;
	circular_list->avail = head;

// Insert Right max_nodes times
	P = circular_list->avail;

	if (P == NULL) {
		asm_balloc_free(c_circular_list_memory);
		return 0;
	}

	CircularNode* P_next = P;

	for (uint32_t i = max_nodes; i > 0; i--){
		P = P_next;
		if (P == NULL) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}

		P->info = i;
		P_next = P->link;
	}

	if (circular_list->ptr == NULL) {
		// List is currently empty, P points to itself
		P->link = circular_list->avail;	// last point to first
		circular_list->ptr = P;
		circular_list->avail = P_next;
	} else {
		// Insert P at the front
		P->link = circular_list->ptr->link;
		circular_list->ptr->link = circular_list->avail;
		circular_list->avail = P_next;
		circular_list->ptr = P;
	}

	circular_list_clear(circular_list);

	asm_balloc_free(c_circular_list_memory);
	return 1;
}
