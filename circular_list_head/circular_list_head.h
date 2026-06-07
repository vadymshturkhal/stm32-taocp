#ifndef CIRCULAR_LIST_HEAD_H
#define CIRCULAR_LIST_HEAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "storage_pool.h"

typedef struct CircularNode {
    struct CircularNode* link;
    uint32_t info;
} CircularNode;

typedef struct CircularNodeHead{
    void* link;
} CircularNodeHead;

typedef struct {
	CircularNodeHead* head;
	Storage_Pool* storage_pool;
} CircularListHead;

// Prototypes
void circular_list_head_init(CircularListHead* circular_list, Storage_Pool* storage_pool);
bool circular_list_head_insert_left(CircularListHead* circular_list, uint32_t info);
uint32_t circular_list_head_pop(bool* pop_is_success, CircularListHead* circular_list);

#endif
