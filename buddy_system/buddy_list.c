#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "buddy_list.h"

uint32_t buddy_list_init(BuddyList* list, BuddyNode* head, uint32_t m) {
	if (list == NULL || head == NULL) return 1;

	list->head = head;
	list->head->LINKF = list->head;
	list->head->LINKB = list->head;
	list->m = m;
	return 0;
}

uint32_t buddy_list_insert(BuddyNode* node, BuddyList* list) {
	if (list == NULL || node == NULL) return 1;
	
	BuddyNode* S = list->head;
	node->LINKF = S->LINKF;
	node->LINKB = S;
	S->LINKF->LINKB = node;
	S->LINKF = node;
	node->TAG = 1;
	node->KVAL = list->m;
	return 0;
}
