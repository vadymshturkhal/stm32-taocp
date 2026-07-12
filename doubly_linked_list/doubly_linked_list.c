#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "doubly_linked_list.h"
#include "storage_pool.h"

// Using Storage Pool

uint32_t doubly_linked_list_init(DoublyLinkedList* doubly_list, Storage_Pool* storage_pool) {
	if (doubly_list == NULL || storage_pool == NULL) return 1;

	DoublyListHead* head = (DoublyListHead*)doubly_list;
	head->link = head;
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
	DoublyListHead* X = doubly_list->head;

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_pop_left(uint32_t* info, DoublyLinkedList* doubly_list) {
	if (doubly_list == NULL) return 1;

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
	DoublyListHead* X = doubly_list->head->left;

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_pop_right(uint32_t* info, DoublyLinkedList* doubly_list) {
	if (doubly_list == NULL) return 1;

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
	if (circular_list == NULL || X == NULL) return 1;

	// P <= Avail
	DoublyNode* P = storage_pool_pop(circular_list->storage_pool);
	if (P == NULL) return 2;	// Overflow

	P->left = X;
	P->right = X->right;
	X->right->left = P;
	X->right = P;

	return 0;
}

uint32_t doubly_linked_list_delete_node(DoublyLinkedList* doubly_list, DoublyNode* X) {
	if (circular_list == NULL || X == NULL) return 1;

	X->left->right = X->right;
	X->right->left = X->left;

	// Avail <= X
	storage_pool_push(circular_list->storage_pool, X);

	return 0;
}

//void doubly_linked_list_clear(DoublyLinkedList* doubly_list) {

//}

//void doubly_linked_list_union(DoublyLinkedList* doubly_list_a, DoublyLinkedList* doubly_list_b) {

//}
