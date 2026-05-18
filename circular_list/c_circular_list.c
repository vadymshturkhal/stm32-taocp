#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_circular_list.h"

CircularNode* init_circular_list_storage_pool(CircularList* circular_list, uint32_t nodes) {
	CircularNode* avail = (CircularNode*)(circular_list + 1);

	// avail->info = size;
	avail->link = NULL;
	nodes--;

	#pragma GCC unroll 4
	while (nodes > 0) {
		CircularNode* tmp = avail+1;
		// tmp->info = size;
		tmp->link = avail;

		avail = tmp;
		nodes--;
	}

	return avail;
}

CircularList* c_create_circular_list(void* memory, uint32_t nodes) {
	// nodes must be greater than 0

	CircularList* circular_list = (CircularList*)memory;
	circular_list->ptr = NULL;

	CircularNode* avail = init_circular_list_storage_pool(circular_list, nodes);
	circular_list->avail = avail;

	return circular_list;
}

bool circular_list_insert_left(CircularList* circular_list, uint32_t info) {
	// return false if Overflow, else true

	// 1 (P <= Avail)
	if (circular_list->avail == NULL) return false;	// Overflow
	CircularNode* P = circular_list->avail;

	// 2
	P->info = info;

	circular_list->avail = circular_list->avail->link;

	// 3
	if (circular_list->ptr == NULL) {
		// List is currently empty, P points to itself
		P->link = P;
		circular_list->ptr = P;
	} else {
		// Insert P at the front
		P->link = circular_list->ptr->link;
		circular_list->ptr->link = P;
	}

	return true;
}

bool circular_list_insert_right(CircularList* circular_list, uint32_t info) {
	if (!circular_list_insert_left(circular_list, info)) return false;

	circular_list->ptr = circular_list->ptr->link;
	return true;
}

uint32_t circular_list_pop(bool* pop_is_success, CircularList* circular_list) {
	// pop left
	// return 0 if Underflow, else P->info
	// input pop_is_success flag must always be true

	if (circular_list->ptr == NULL) {
		*pop_is_success = false;	// Underflow
		return 0;
	}

	CircularNode* P = circular_list->ptr->link;
//	uint32_t Y = P->info;

	if (circular_list->ptr == P) {
		circular_list->ptr = NULL;
	} else {
		circular_list->ptr->link = P->link;
	}

	P->link = circular_list->avail;
	circular_list->avail = P;

	return P->info;
//	return Y;
}

void circular_list_clear(CircularList* circular_list) {
	if (circular_list->ptr != NULL) {
		CircularNode* P = circular_list->avail;
		circular_list->avail = circular_list->ptr->link;
		circular_list->ptr->link = P;
		circular_list->ptr = NULL;
	}
}

void circular_list_union(CircularList* circular_list_a, CircularList* circular_list_b) {
	// Insert the entire circular_list_b at the right of circular circular_list_a list1
	// Implicitly reduce size of the circular_list_b

	if (circular_list_b->ptr == NULL) return;

	if (circular_list_a->ptr != NULL) {
		CircularNode* P = circular_list_a->ptr->link;
		circular_list_a->ptr->link = circular_list_b->ptr->link;
		circular_list_b->ptr->link = P;
	}

	circular_list_a->ptr = circular_list_b->ptr;
	circular_list_b->ptr = NULL;
}
