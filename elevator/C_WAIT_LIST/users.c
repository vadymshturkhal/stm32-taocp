#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "users.h"
#include "values.h"

uint32_t users_init(Users* users, SharedState* shared_state, Storage_Pool* storage_pool) {
	if (users == NULL || shared_state == NULL || storage_pool == NULL) return 1;

	users->shared_state = shared_state;
	shared_state->users = users;   // add Users to shared state

	users->storage_pool = storage_pool;

	users->USER1 = storage_pool_pop(storage_pool);
	if (users->USER1 == NULL) return 2;

	users->USER1->NEXTINST = U1;  // Set NEXTINST

	users->user_id = 0;

	return 0;
}

void users_start(SharedState* shared_state) {
	// USER1 node represents action U1 and it is initially the sole entry in the WAIT list
	immed(shared_state, shared_state->users->USER1);
}

// [Enter, prepare for successor]
void U1(SharedState* shared_state, ElevatorNode* C) {
	// User fabric

	Users* users = shared_state->users;

	// 1 JMP VALUES
	Values user_values = values();

	// 3 Create User
	ElevatorNode* user = storage_pool_pop(users->storage_pool);
	if (user == NULL) return;

	user->IN = user_values.IN;
	user->OUT = user_values.OUT;
	user->GIVEUPTIME = user_values.GIVEUPTIME;

	// 4 increment user_id (not in Coroutine U)
	users->user_id += 1;

	// 2 LDA INTERTIME (time before another user enters) / JMP HOLD
	hold(shared_state, C, user_values.INTERTIME);

	// TODO: trace print (Table 1 style) -- no logging yet

	// 5 to U2, with C now the new node
	U2(shared_state, user);
}

// [Signal and wait]
void U2(SharedState* shared_state, ElevatorNode* user) {

}

void U5(SharedState* shared_state, ElevatorNode* C) {

}

void U6(SharedState* shared_state, ElevatorNode* C) {

}
