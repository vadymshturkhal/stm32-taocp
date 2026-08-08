#include <stdint.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"

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
