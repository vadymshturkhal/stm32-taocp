#ifndef BUDDY_LIST_H
#define BUDDY_LIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BuddyNode {
    struct BuddyNode* LINKF;
    struct BuddyNode* LINKB;
    uint8_t             TAG;
    uint8_t             KVAL;
} BuddyNode;

typedef struct {
    BuddyNode* head;
    uint8_t     m;
} BuddyList;

uint32_t buddy_list_init(BuddyList* list, BuddyNode* head, uint32_t m);
uint32_t buddy_list_insert(BuddyList* list, BuddyNode* node);

#endif
