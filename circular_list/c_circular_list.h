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
uint32_t circular_list_pop(CircularList* circular_list, bool* pop_is_success);
void circular_list_clear(CircularList* circular_list);
void circular_list_union(CircularList* circular_list_a, CircularList* circular_list_b);

#endif
