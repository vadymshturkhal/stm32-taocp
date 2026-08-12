#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "users.h"

uint32_t users_init(Users* users, SharedState* shared_state, Storage_Pool* storage_pool) {
	if (users == NULL || shared_state == NULL || storage_pool == NULL) return 1;

	users->shared_state = shared_state;
	shared_state->users = users;   // add Users to shared state

	users->USER1 = storage_pool_pop(storage_pool);
	if (users->USER1 == NULL) return 2;

	users->USER1->NEXTINST = U1;  // Set NEXTINST

	users->user_id = 0;

	return 0;
}

void U1(SharedState* shared_state, ElevatorNode* C) {

}

void U5(SharedState* shared_state, ElevatorNode* C) {

}

void U6(SharedState* shared_state, ElevatorNode* C) {

}
