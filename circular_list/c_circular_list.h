// c_circular_list.h
#ifndef C_CIRCULAR_LIST_H	// prevent duplicate definitions
#define C_CIRCULAR_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct CircularNode {
    uint32_t info;
    struct CircularNode* link;
} CircularNode;

typedef struct {
	CircularNode* ptr;
	CircularNode* avail;
} CircularList;

// Prototypes
CircularNode* init_circular_list_storage_pool(CircularList* circular_list, uint32_t size);
CircularList* c_create_circular_list(void* memory, uint32_t size);

bool circular_list_insert_left(CircularList* circular_list, uint32_t info);
bool circular_list_insert_right(CircularList* circular_list, uint32_t info);
uint32_t circular_list_pop(bool* pop_is_success, CircularList* circular_list);
void circular_list_clear(CircularList* circular_list);
void circular_list_union(CircularList* circular_list_a, CircularList* circular_list_b);

static inline bool circular_list_insert_left_inline(CircularList* circular_list, uint32_t info) {
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

static inline bool circular_list_insert_right_inline(CircularList* circular_list, uint32_t info) {
	if (!circular_list_insert_left_inline(circular_list, info)) return false;

	circular_list->ptr = circular_list->ptr->link;
	return true;
}

static inline uint32_t circular_list_pop_inline(bool* pop_is_success, CircularList* circular_list) {
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

#endif
