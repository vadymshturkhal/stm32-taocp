#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stdint.h>

#include "buddy_list.h"

// NOTE:
// Add ASSERT(_end <= _stack_limit, "RAM: .bss overruns _stack_limit") to .ld file, after RAM setting

// Arena size (2**BUDDY_M), and BUDDY_M is 14 for G431RB and allocates 16KB
#ifndef BUDDY_M
#define BUDDY_M 14
#endif

typedef struct {
    BuddyList* list;
    uint8_t*   base; // memory pointer
    uint16_t   m;    // 2**14 is max for G431RB
} BuddySystem;

void* buddy_alloc(size_t size);
uint32_t buddy_free(void* ptr);
#endif
