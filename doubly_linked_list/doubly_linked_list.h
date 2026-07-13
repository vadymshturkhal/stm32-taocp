#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "storage_pool.h"

typedef struct DoublyNode {
    struct DoublyNode* left;
    struct DoublyNode* right;
    uint32_t info;
} DoublyNode;

typedef struct DoublyListHead{
	DoublyNode* right;
	DoublyNode* left;
} DoublyListHead;

typedef struct {
	DoublyNode* head;
	Storage_Pool* storage_pool;
	uint32_t size;
} DoublyLinkedList;

// Prototypes
uint32_t doubly_linked_list_init(DoublyLinkedList* doubly_list, Storage_Pool* storage_pool);
uint32_t doubly_linked_list_insert_left(DoublyLinkedList* doubly_list, uint32_t info);
uint32_t doubly_linked_list_pop_left(uint32_t* info, DoublyLinkedList* doubly_list);
uint32_t doubly_linked_list_insert_right(DoublyLinkedList* doubly_list, uint32_t info);
uint32_t doubly_linked_list_pop_right(uint32_t* info, DoublyLinkedList* doubly_list);
uint32_t doubly_linked_list_insert_node(DoublyLinkedList* doubly_list, DoublyNode* X);
uint32_t doubly_linked_list_delete_node(DoublyLinkedList* doubly_list, DoublyNode* X);

// TODO
//void doubly_linked_list_clear(DoublyLinkedList* doubly_list);
//void doubly_linked_list_union(DoublyLinkedList* doubly_list_a, DoublyLinkedList* doubly_list_b);

#endif
