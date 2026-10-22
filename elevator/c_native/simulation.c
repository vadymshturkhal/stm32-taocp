#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator_list.h"
#include "elevator.h"
#include "users.h"
#include "values.h"
#include "simulation.h"

uint32_t start_elevator_simulation(uint32_t users_quantity) {
	values_seed(1);  // fixed seed: reproducible runs, matches Python's random.seed(1)

	// Not in MIX -- max_concurrent_users bounds how many arriving users can be
	// in flight (queued/riding) at once; it's independent of users_quantity
	// (the total to generate over the whole run), since nodes get recycled
	// via storage_pool_push in U6 as each user completes their journey
	uint32_t max_concurrent_users = 16;
	uint32_t num_lists = FLOORS + 2;  // ELEVATOR_LIST, QUEUE[0..FLOORS-1], WAIT_LIST -- each pops a head node from the pool
	uint32_t max_nodes = num_lists + 4 + max_concurrent_users;  // list heads, ELEV1, ELEV2, ELEV3, USER1, and max_concurrent_users

	uint32_t storage_pool_size = sizeof(Storage_Pool) + (max_nodes) * sizeof(ElevatorNode);
	uint32_t shared_state_size = sizeof(SharedState);  // ElevatorList structs (head pointers) reserved in shared state; head nodes themselves come from storage_pool
	uint32_t elevator_size = sizeof(Elevator);
	uint32_t users_size = sizeof(Users);
	uint32_t master_memory_size = storage_pool_size + shared_state_size + elevator_size + users_size;

	// Not in MIX -- STM32 build uses asm_balloc (a static-arena allocator);
	// this PC build just uses malloc since there's no embedded RAM budget to manage
	void* master_memory = malloc(master_memory_size);
	if (master_memory == NULL) return 1;

	Storage_Pool* storage_pool = create_storage_pool(master_memory, sizeof(ElevatorNode), max_nodes);
	if (storage_pool == NULL) return 2;

	uint8_t* shared_state_memory = (uint8_t*)master_memory + storage_pool_size;
	uint8_t* elevator_memory = (uint8_t*)shared_state_memory + shared_state_size;
	uint8_t* users_memory = (uint8_t*)elevator_memory + elevator_size;

	SharedState* shared_state = (SharedState*)shared_state_memory;
	Elevator* elevator = (Elevator*)elevator_memory;
	Users* users = (Users*)users_memory;

	uint32_t status = shared_state_init(shared_state, storage_pool);
	if (status != 0) {
		free(master_memory);
		return status;
	}

	status = elevator_init(elevator, shared_state, storage_pool);
	if (status != 0) {
		free(master_memory);
		return status;
	}

	status = users_init(users, shared_state, storage_pool, users_quantity);
	if (status != 0) {
		free(master_memory);
		return status;
	}

	// USER1 node represents action U1 and it is initially the sole entry in the WAIT list
	users_start(shared_state);

	// CYCLE control routine: run until the WAIT list runs dry
	cycle(shared_state);

	free(master_memory);
	return 0;
}
