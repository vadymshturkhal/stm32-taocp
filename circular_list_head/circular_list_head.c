#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "storage_pool.h"
#include "circular_list_head.h"


// Using Storage Pool

void circular_list_head_init(CircularListHead* circular_list, Storage_Pool* storage_pool) {
	CircularNodeHead* head = (CircularNodeHead*)circular_list;
	head->link = head;
	circular_list->storage_pool = storage_pool;
}

bool circular_list_head_insert_left(CircularListHead* circular_list, uint32_t info) {
	// return false if Overflow, else true
	if (circular_list == NULL) return false;

	// 1 (P <= Avail)
	CircularNode* P = storage_pool_pop(circular_list->storage_pool);
	if (P == NULL) return false;	// Overflow

	// 2
	P->info = info;

	// 3 Insert P at the front
	P->link = (CircularNode*)circular_list->head->link;
	circular_list->head->link = P;

	return true;
}

uint32_t circular_list_head_pop(bool* pop_is_success, CircularListHead* circular_list) {
	// pop left
	// return 0 if Underflow, else P->info

	if (circular_list->head == circular_list->head->link) {
		*pop_is_success = false;	// Underflow
		return 0;
	}

	*pop_is_success = true;

	CircularNode* P = circular_list->head->link;
	circular_list->head->link = P->link;

	uint32_t info = P->info;
	storage_pool_push(circular_list->storage_pool, P);
	return info;
}


// O(N)
//void circular_list_head_clear(CircularListHead* circular_list) {
//}

// O(N)
//void storage_circular_list_union(CircularListHead* circular_list_a, CircularListHead* circular_list_b) {
//}
