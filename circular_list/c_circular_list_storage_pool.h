// c_circular_list_storage_pool.h
#ifndef C_CIRCULAR_LIST_H	// prevent duplicate definitions
#define C_CIRCULAR_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "storage_pool.h"

typedef struct CircularNode {
    uint32_t info;
    struct CircularNode* link;
} CircularNode;


typedef struct {
	CircularNode* ptr;
	Storage_Pool* storage_pool;
} CircularListStorage;

// Prototypes
void storage_init_circular_list(CircularListStorage* circular_list, Storage_Pool* storage_pool);
bool storage_circular_list_insert_left(CircularListStorage* circular_list, uint32_t info);
bool storage_circular_list_insert_right(CircularListStorage* circular_list, uint32_t info);
uint32_t storage_circular_list_pop(bool* pop_is_success, CircularListStorage* circular_list);
void storage_circular_list_clear(CircularListStorage* circular_list);
void storage_circular_list_union(CircularListStorage* circular_list_a, CircularListStorage* circular_list_b);

#endif
