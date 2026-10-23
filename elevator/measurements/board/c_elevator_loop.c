/* c_elevator_loop.c - the C elevator in the main loop, one node per pass
 *
 * c_elevator_start() is start_elevator_simulation() up to users_start();
 * c_elevator_step() is one pass of cycle()'s while loop;
 * c_elevator_stop() is its final free(). The functions they call are the
 * measured C elevator, unchanged.
 */
#include <stdint.h>
#include <stdlib.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator_list.h"
#include "elevator.h"
#include "users.h"
#include "values.h"
#include "c_elevator_loop.h"

static void* master_memory = NULL;
static SharedState* shared_state = NULL;

uint32_t c_elevator_start(uint32_t max_users) {
	values_seed(1);  // fixed seed: the same users as every other run

	uint32_t num_lists = FLOORS + 2;  // ELEVATOR_LIST, QUEUE[0..FLOORS-1], WAIT_LIST
	uint32_t max_nodes = num_lists + 4 + max_users;  // list heads, ELEV1, ELEV2, ELEV3, USER1, and max_users

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(ElevatorNode);
	uint32_t shared_state_size = sizeof(SharedState);
	uint32_t elevator_size = sizeof(Elevator);
	uint32_t users_size = sizeof(Users);
	uint32_t master_memory_size = storage_pool_size + shared_state_size + elevator_size + users_size;

	master_memory = malloc(master_memory_size);
	if (master_memory == NULL) return 1;

	Storage_Pool* storage_pool = create_storage_pool(master_memory, sizeof(ElevatorNode), max_nodes);
	if (storage_pool == NULL) {
		c_elevator_stop();
		return 2;
	}

	uint8_t* shared_state_memory = (uint8_t*)master_memory + storage_pool_size;
	uint8_t* elevator_memory = shared_state_memory + shared_state_size;
	uint8_t* users_memory = elevator_memory + elevator_size;

	shared_state = (SharedState*)shared_state_memory;
	Elevator* elevator = (Elevator*)elevator_memory;
	Users* users = (Users*)users_memory;

	uint32_t status = shared_state_init(shared_state, storage_pool);
	if (status == 0) status = elevator_init(elevator, shared_state, storage_pool);
	if (status == 0) status = users_init(users, shared_state, storage_pool, max_users);
	if (status != 0) {
		c_elevator_stop();
		return status;
	}

	// USER1 node represents action U1 and it is initially the sole entry in the WAIT list
	users_start(shared_state);
	return 0;
}

int c_elevator_step(void) {
	if (shared_state == NULL) return 0;

	// One pass of cycle(): is the WAIT list empty?
	ElevatorNode* head = shared_state->WAIT_LIST.head;
	ElevatorNode* C = head->right1;
	if (C == head) return 0;

	// Advance TIME to its NEXTTIME, unlink it, call NEXTINST
	shared_state->TIME = C->NEXTTIME;
	elevator_list_delete_nodew(C);
	C->NEXTINST(shared_state, C);
	return 1;
}

uint32_t c_elevator_time(void) {
	return (shared_state != NULL) ? shared_state->TIME : 0;
}

void c_elevator_stop(void) {
	free(master_memory);
	master_memory = NULL;
	shared_state = NULL;
}
