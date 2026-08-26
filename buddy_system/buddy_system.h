#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stdint.h>

#include "buddy_list.h"

#define BUDDY_M 14  // Arena size (2**BUDDY_M), and BUDDY_M is 14 for G431RB, and allocates 16KB

typedef struct {
    BuddyList* list;
    uint8_t*   base; // memory pointer
    uint16_t   m;    // 2**14 is max for G431RB
} BuddySystem;

uint32_t buddy_system_init(BuddySystem* system);
BuddyNode* buddy_address(BuddySystem* system, BuddyNode* node, uint32_t k);
void* buddy_alloc(BuddySystem* system, uint32_t size);
uint32_t buddy_free(BuddySystem* system, void* ptr, uint32_t size);
void* buddy_system_reservation(BuddySystem* system, uint32_t k);
uint32_t buddy_system_liberation(BuddySystem* system, void* L, uint32_t k);

#endif
