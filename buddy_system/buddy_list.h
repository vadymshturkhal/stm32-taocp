#ifndef BUDDY_LIST_H
#define BUDDY_LIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BuddyNode {
    struct BuddyNode* LINKF;
    struct BuddyNode* LINKB;
    bool                TAG;
    uint16_t            KVAL;
} BuddyNode;

typedef struct {
    BuddyNode* head;
    uint32_t    m;
} BuddyList;

uint32_t buddy_list_init(BuddyList* list, BuddyNode* head, uint32_t m);
uint32_t buddy_list_insert(BuddyNode* node, BuddyList* list);
uint32_t buddy_list_remove(BuddyNode* node);

#endif
