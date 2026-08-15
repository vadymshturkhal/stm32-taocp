#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator_list.h"
#include "elevator.h"
#include "users.h"
#include "values.h"
#include "main.h"

// NOTE: USER1 and ELEV's nodes are ElevatorNode, can change to special node with two links fields according to MIX

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
uint32_t ASM_ELEVATOR_INIT(Elevator* elevator, SharedState* shared_state, Storage_Pool* storage_pool);
uint32_t ASM_USERS_INIT(Users* users, SharedState* shared_state, Storage_Pool* storage_pool, uint32_t users_quantity);
extern void ASM_USERS_START(SharedState* shared_state);
extern void ASM_CYCLE_START(SharedState* shared_state);

uint32_t start_asm_elevator_simulation(uint32_t max_users) {
	values_seed(1);  // fixed seed: reproducible runs, matches Python's random.seed(1)

	uint32_t num_lists = FLOORS + 2;  // ELEVATOR_LIST, QUEUE[0..FLOORS-1], WAIT_LIST -- each pops a head node from the pool
	uint32_t max_nodes = num_lists + 4 + max_users;  // list heads, ELEV1, ELEV2, ELEV3, USER1, and max_users

	uint32_t storage_pool_size = sizeof(Storage_Pool) + (max_nodes) * sizeof(ElevatorNode);
	uint32_t shared_state_size = sizeof(SharedState);  // ElevatorList structs (head pointers) reserved in shared state; head nodes themselves come from storage_pool
	uint32_t elevator_size = sizeof(Elevator);
	uint32_t users_size = sizeof(Users);
	uint32_t master_memory_size = storage_pool_size + shared_state_size + elevator_size + users_size;

	void* master_memory = asm_balloc(master_memory_size);
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
		asm_balloc_free(master_memory);
		return status;
	}

	status = ASM_ELEVATOR_INIT(elevator, shared_state, storage_pool);
	if (status != 0) {
		asm_balloc_free(master_memory);
		return status;
	}

	status = ASM_USERS_INIT(users, shared_state, storage_pool, max_users);
	if (status != 0) {
		asm_balloc_free(master_memory);
		return status;
	}

	ASM_USERS_START(shared_state);
	ASM_CYCLE_START(shared_state);

	asm_balloc_free(master_memory);
	return 0;
}
