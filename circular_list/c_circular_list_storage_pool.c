#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "storage_pool.h"
#include "c_circular_list_storage_pool.h"

void storage_init_circular_list(CircularListStorage* circular_list, Storage_Pool* storage_pool) {
	circular_list->ptr = NULL;
	circular_list->storage_pool = storage_pool;
}

bool storage_circular_list_insert_left(CircularListStorage* circular_list, uint32_t info) {
	// return false if Overflow, else true
	if (circular_list == NULL) return false;

	// 1 (P <= Avail)
	CircularNode* P = storage_pool_pop(circular_list->storage_pool);
	if (P == NULL) return false;	// Overflow

	// 2
	P->info = info;

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

bool storage_circular_list_insert_right(CircularListStorage* circular_list, uint32_t info) {
	if (!storage_circular_list_insert_left(circular_list, info)) return false;

	circular_list->ptr = circular_list->ptr->link;
	return true;
}

uint32_t storage_circular_list_pop(bool* pop_is_success, CircularListStorage* circular_list) {
	// pop left
	// return 0 if Underflow, else P->info
	// input pop_is_success flag must always be true

	if (circular_list->ptr == NULL) {
		*pop_is_success = false;	// Underflow
		return 0;
	}

	CircularNode* P = circular_list->ptr->link;

	if (circular_list->ptr == P) {
		circular_list->ptr = NULL;
	} else {
		circular_list->ptr->link = P->link;
	}

	uint32_t info = P->info;
	storage_pool_push(circular_list->storage_pool, P);
	return info;
}

void storage_circular_list_clear(CircularListStorage* circular_list) {
	if (circular_list == NULL) return;

	if (circular_list->ptr != NULL) {
		CircularNode* head = circular_list->ptr->link;
		CircularNode* tail = circular_list->ptr;

		storage_pool_add_slice(circular_list->storage_pool, head, tail);
		circular_list->ptr = NULL;
	}
}

void storage_circular_list_union(CircularListStorage* circular_list_a, CircularListStorage* circular_list_b) {
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
