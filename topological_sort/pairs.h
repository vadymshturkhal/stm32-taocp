#ifndef PAIRS_H
#define PAIRS_H

#include <stdint.h>

typedef struct {
	uint32_t j;
	uint32_t k;
} Pair;

typedef struct TopologicalNode {
    uint32_t succ;
    struct TopologicalNode* next;
} TopologicalNode;

// Declaration
TopologicalNode* init_avail_list(void* memory, uint32_t size);

#endif
