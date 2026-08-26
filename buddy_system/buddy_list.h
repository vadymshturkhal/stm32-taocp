#ifndef BUDDY_LIST_H
#define BUDDY_LIST_H

#include <stdint.h>
#include <stddef.h>

typedef struct BuddyNode {
    uint8_t                TAG;
    // NOTE: KVAL is only at available blocks in TAOCP, but we pay 3 bytes paddings already
    uint16_t           KVAL;   // field to specify k when their size is 2**k, 2**14 is max for G431RB
    struct BuddyNode* LINKF;
    struct BuddyNode* LINKB;

} BuddyNode;

// Available memory starts after TAG
#define BUDDY_HEADER offsetof(BuddyNode, LINKF)

typedef struct {
    BuddyNode* head;
    uint32_t    m;
} BuddyList;

uint32_t buddy_list_init(BuddyList* list, BuddyNode* head, uint32_t m);
uint32_t buddy_list_insert(BuddyNode* node, BuddyList* list);
uint32_t buddy_list_remove(BuddyNode* node);

#endif
