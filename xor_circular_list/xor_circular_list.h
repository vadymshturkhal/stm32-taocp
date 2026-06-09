#ifndef XOR_CIRCULAR_LIST_H
#define XOR_CIRCULAR_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "storage_pool.h"

typedef struct {
	uintptr_t link;
    uint32_t info;
} XORCircularNode;

typedef struct {
    uintptr_t link;
} XORCircularNodeHead;

typedef struct XORCircularList {
	XORCircularNodeHead* head1;
	XORCircularNodeHead* head2;
	Storage_Pool* storage_pool;
	uint32_t size;
} XORCircularList;

// Prototypes
void xor_circular_list_init(XORCircularList* circular_list, Storage_Pool* storage_pool);
bool xor_circular_list_insert_left(XORCircularList* circular_list, uint32_t info);
uint32_t xor_circular_list_pop_left(bool* ok, XORCircularList* list);

#endif
