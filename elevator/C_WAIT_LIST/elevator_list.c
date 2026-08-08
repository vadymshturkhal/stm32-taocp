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
