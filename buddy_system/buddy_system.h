#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stdint.h>

#include "buddy_list.h"

typedef struct {
    BuddyList* list;
    uint8_t*   base; // memory pointer
    uint16_t   m;    // 2**14 is max for G431RB
} BuddySystem;

uint32_t buddy_system_init(BuddySystem* system, void* memory, uint32_t m);
BuddyNode* buddy_address(BuddySystem* system, BuddyNode* node, uint32_t k);
void* buddy_system_reservation(BuddySystem* system, uint32_t k);
uint32_t buddy_system_liberation(BuddySystem* system, void* L, uint32_t k);

#endif
