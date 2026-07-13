#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "storage_pool.h"
#include "doubly_linked_list.h"

// Using Storage Pool

uint32_t doubly_linked_list_init(DoublyLinkedList* doubly_list, Storage_Pool* storage_pool) {
	if (doubly_list == NULL || storage_pool == NULL) return 1;

	DoublyNode* head = storage_pool_pop(storage_pool);
	if (head == NULL) return 2;	// Overflow

	doubly_list->head = head;
	doubly_list->head->left = doubly_list->head;
	doubly_list->head->right = doubly_list->head;

	doubly_list->size = 0;
	doubly_list->storage_pool = storage_pool;
	return 0;
}

uint32_t doubly_linked_list_insert_left(DoublyLinkedList* doubly_list, uint32_t info) {
	if (doubly_list == NULL) return 1;

	// 1 (P <= Avail)
	DoublyNode* P = storage_pool_pop(doubly_list->storage_pool);
	if (P == NULL) return 2;	// Overflow

	// 2
	P->info = info;

	// Increment size
	doubly_list->size++;

	// 3 Insert P at Front
	DoublyNode* X = doubly_list->head;

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_pop_left(uint32_t* info, DoublyLinkedList* doubly_list) {
	if (doubly_list == NULL || info == NULL) return 1;

	if (doubly_list->head == doubly_list->head->right) {
		return 2;  // Underflow
	}

    // Decrement size
	doubly_list->size -= 1;

    DoublyNode* X = doubly_list->head->right;
	X->left->right = X->right;
	X->right->left = X->left;

	*info = X->info;
	storage_pool_push(doubly_list->storage_pool, X);
	return 0;
}

uint32_t doubly_linked_list_insert_right(DoublyLinkedList* doubly_list, uint32_t info) {
	if (doubly_list == NULL) return 1;

	// 1 (P <= Avail)
	DoublyNode* P = storage_pool_pop(doubly_list->storage_pool);
	if (P == NULL) return 2;	// Overflow

	// 2
	P->info = info;

	// Increment size
	doubly_list->size++;

	// 3 Insert P at Rear
	DoublyNode* X = doubly_list->head->left;

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_pop_right(uint32_t* info, DoublyLinkedList* doubly_list) {
	if (doubly_list == NULL || info == NULL) return 1;

	if (doubly_list->head == doubly_list->head->right) {
		return 2;  // Underflow
	}

    // Decrement size
	doubly_list->size -= 1;

    DoublyNode* X = doubly_list->head->left;
	X->left->right = X->right;
	X->right->left = X->left;

	*info = X->info;
	storage_pool_push(doubly_list->storage_pool, X);
	return 0;
}

uint32_t doubly_linked_list_insert_node(DoublyLinkedList* doubly_list, DoublyNode* X) {
	if (doubly_list == NULL || X == NULL) return 1;

	// P <= Avail
	DoublyNode* P = storage_pool_pop(doubly_list->storage_pool);
	if (P == NULL) return 2;	// Overflow

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_delete_node(DoublyLinkedList* doubly_list, DoublyNode* X) {
	if (doubly_list == NULL || X == NULL) return 1;

	X->left->right = X->right;
	X->right->left = X->left;

	// Avail <= X
	storage_pool_push(doubly_list->storage_pool, X);

	return 0;
}

uint32_t doubly_linked_list_clear(DoublyLinkedList* list) {
	if (list->head->left != list->head) {
		// storage pool used node->left as a link
		DoublyNode* head = list->head->left;
		DoublyNode* tail = list->head->right;

		storage_pool_add_slice(list->storage_pool, head, tail);
		list->head->left = list->head;
		list->head->right = list->head;

		list->storage_pool->size += list->size;
		list->size = 0;

		return 0;
	}

	return 1;
}

uint32_t doubly_linked_list_union(DoublyLinkedList* list_a, DoublyLinkedList* list_b) {
	// Insert the list_b at right of list_a
	// Reduce the size of the list_b

	if (list_a == NULL || list_b == NULL) return 1;

	if (list_b->head->left != list_b->head) {
		DoublyNode* list_b_begin = list_b->head->right;
		DoublyNode* list_b_end = list_b->head->left;

		DoublyNode* list_a_end = list_a->head->left;

		// link end with start
		list_a_end->right = list_b_begin;
		list_b_begin->left = list_a_end;

		// link all with head
		list_b_end->right = list_a->head;
		list_a->head->left = list_b_end;

		// Empty list_b
		list_b->head->left = list_b->head;
		list_b->head->right = list_b->head;
		list_a->size += list_b->size;
		list_b->size = 0;

		return 0;
	}

	return 2;
}



//void circular_list_union(CircularList* circular_list_a, CircularList* circular_list_b) {
//	// Insert the entire circular_list_b at the right of circular circular_list_a list1
//	// Implicitly reduce size of the circular_list_b
//
//	if (circular_list_b->ptr == NULL) return;
//
//	if (circular_list_a->ptr != NULL) {
//		CircularNode* P = circular_list_a->ptr->link;
//		circular_list_a->ptr->link = circular_list_b->ptr->link;
//		circular_list_b->ptr->link = P;
//	}
//
//	circular_list_a->ptr = circular_list_b->ptr;
//	circular_list_b->ptr = NULL;
//}
