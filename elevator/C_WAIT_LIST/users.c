#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator.h"
#include "users.h"
#include "values.h"

void U3(SharedState* shared_state, ElevatorNode* user);
void U4(SharedState* shared_state, ElevatorNode* user);

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
	Elevator* elevator = shared_state->elevator;

	// 1. If FLOOR == IN
	if (elevator->FLOOR == user->IN) {
		// 2. And if the elevator's next action is step E6
		// (if the elevator doors are now closing)
		if (elevator->ELEV1->NEXTINST == E6) {
			// Reposition it at E3
			elevator->ELEV1->NEXTINST = E3;

			// JMP DELETEW
			elevator_list_delete_nodew(elevator->ELEV1);

			// JMP IMMED
			immed(shared_state, elevator->ELEV1);

			// JMP U3
			U3(shared_state, user);
			return;
		}

		// 3. If D3 != 0
		if (elevator->D3 != 0) {
			// Set D3 = 0, D1 to a nonzero value and start up the elevator's activity E4 again

			elevator->D1 = 1;
			elevator->D3 = 0;

			// JMP IMMED
			immed(shared_state, elevator->ELEV1);

			// JMP U3
			U3(shared_state, user);
			return;
		}
	}

	// 4. In all other cases the user sets CALLUP[IN] = 1 or CALLDOWN[IN] = 1
	// according as OUT > IN or OUT < IN
	if (user->OUT > user->IN) {
		shared_state->CALLS[user->IN] |= CALLUP;
	} else {
		shared_state->CALLS[user->IN] |= CALLDOWN;
	}

	// 5. And if D2 == 0 or the elevator in its "dormant" position E1, DECISION is performed
	if (elevator->D2 == 0 || elevator->ELEV1->NEXTINST == E1) {
		decision(shared_state, U2);
	}

	U3(shared_state, user);
}

// [Enter queue]
void U3(SharedState* shared_state, ElevatorNode* user) {

}

// [Wait GIVEUPTIME units]
static void U4A(SharedState* shared_state, ElevatorNode* user) {
	// LDA GIVEUPTIME
	// JMP HOLDC
	uint32_t delay = user->GIVEUPTIME;
	holdc(shared_state, user, delay, U4);
}

// [Give up]
void U4(SharedState* shared_state, ElevatorNode* user) {

}

// [Get in]
void U5(SharedState* shared_state, ElevatorNode* C) {

}

// [Get out]
void U6(SharedState* shared_state, ElevatorNode* C) {

}
