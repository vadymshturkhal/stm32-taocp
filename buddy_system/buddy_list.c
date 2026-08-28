#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "buddy_list.h"

// Doubly Linked Circular List

uint32_t buddy_list_init(BuddyList* list, BuddyNode* head, uint32_t m) {
	if (list == NULL || head == NULL) return 1;

	list->head = head;
	list->head->LINKF = list->head;
	list->head->LINKB = list->head;
	list->head->TAG = 0;
	list->head->KVAL = m;

	list->m = m;
	return 0;
}

uint32_t buddy_list_insert(BuddyNode* node, BuddyList* list) {
	if (list == NULL || node == NULL) return 1;
	
	BuddyNode* P = list->head;
	node->LINKF = P->LINKF;
	node->LINKB = P;
	P->LINKF->LINKB = node;
	P->LINKF = node;
	node->TAG = 1;
	node->KVAL = list->m;
	return 0;
}

uint32_t buddy_list_remove(BuddyNode* node) {
	// Removes node from it's list
	if (node == NULL) return 1;

	node->LINKB->LINKF = node->LINKF;
	node->LINKF->LINKB = node->LINKB;
	return 0;
}
