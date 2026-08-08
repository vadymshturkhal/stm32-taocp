#include <stdint.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_list.h"

uint32_t elevator_list_init(ElevatorList* elevator_list, Storage_Pool* storage_pool) {
	if (elevator_list == NULL || storage_pool == NULL) return 1;

	ElevatorNode* head = storage_pool_pop(storage_pool);
	if (head == NULL) return 2;

	elevator_list->head = head;
	elevator_list->head->left1 = elevator_list->head;
	elevator_list->head->right1 = elevator_list->head;
	elevator_list->head->left2 = elevator_list->head;
	elevator_list->head->right2 = elevator_list->head;

	elevator_list->storage_pool = storage_pool;
	return 0;
}

void insert_node_at_frontw(ElevatorList* elevator_list, ElevatorNode* node) {
    // Insert node into WAIT list using LLINK1 and RLINK1

    ElevatorNode* X = elevator_list->head;
    node->left1 = X;
    node->right1 = X->right1;
    X->right1->left1 = node;
    X->right1 = node;
}

void insert_node_at_rear(ElevatorList* elevator_list, ElevatorNode* node) {
	ElevatorNode* Q = elevator_list->head->left2;
	node->left2 = Q;
	elevator_list->head->left2 = node;
	Q->right2 = node;
	node->right2 = elevator_list->head;
}
