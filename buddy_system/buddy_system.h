#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stdint.h>

#include "buddy_list.h"

// NOTE: Add ASSERT(_end <= _stack_limit, "RAM: .bss overruns _stack_limit") 
// to.ld, after setting RAM

// Arena size (2**BUDDY_M), and BUDDY_M is 14 for G431RB and allocates 16KB
#ifndef BUDDY_M
#define BUDDY_M 14
#endif

typedef struct {
    BuddyList* list;
    uint8_t*   base; // memory pointer
    uint16_t   m;    // 2**14 is max for G431RB
} BuddySystem;

uint32_t buddy_system_init(BuddySystem* system);
BuddyNode* buddy_address(BuddySystem* system, BuddyNode* node, uint32_t k);
void* buddy_alloc(BuddySystem* system, uint32_t size);
uint32_t buddy_free(BuddySystem* system, void* ptr);
void* buddy_system_reservation(BuddySystem* system, uint32_t k);
uint32_t buddy_system_liberation(BuddySystem* system, void* L, uint32_t k);

#endif
