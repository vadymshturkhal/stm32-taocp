#include <stdint.h>

#include "elevator_settings.h"
#include "main.h"

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

uint32_t shared_state_init(SharedState* shared_state, Storage_Pool* storage_pool) {
	if (shared_state == NULL || storage_pool == NULL) return 1;

	shared_state->TIME = 0;
	for (uint32_t i = 0; i < FLOORS; i++) {
		shared_state->CALLS[i] = 0;
	}

	uint32_t status;

	status = elevator_list_init(&shared_state->ELEVATOR_LIST, storage_pool);
	if (status != 0) return status;

	for (uint32_t i = 0; i < FLOORS; i++) {
		status = elevator_list_init(&shared_state->QUEUE[i], storage_pool);
		if (status != 0) return status;
	}

	status = elevator_list_init(&shared_state->WAIT_LIST, storage_pool);
	if (status != 0) return status;

	shared_state->WAIT_LIST.head->NEXTTIME = 0;  // set NEXTTIME of head node to 0

	shared_state->elevator = NULL;

	return 0;
}
