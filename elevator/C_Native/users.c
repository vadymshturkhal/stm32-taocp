#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator.h"
#include "users.h"
#include "values.h"
#include "trace.h"

// Private functions
static void U1(SharedState* shared_state, ElevatorNode* C);
static void U2(SharedState* shared_state, ElevatorNode* user);
static void U3(SharedState* shared_state, ElevatorNode* user);
static void U4(SharedState* shared_state, ElevatorNode* user);
static void U4A(SharedState* shared_state, ElevatorNode* user);
static void U6_impl(SharedState* shared_state, ElevatorNode* user, bool is_print);

uint32_t users_init(Users* users, SharedState* shared_state, Storage_Pool* storage_pool, uint32_t users_quantity) {
	if (users == NULL || shared_state == NULL || storage_pool == NULL) return 1;

	users->shared_state = shared_state;
	shared_state->users = users;   // add Users to shared state

	users->storage_pool = storage_pool;

	users->USER1 = storage_pool_pop(storage_pool);
	if (users->USER1 == NULL) return 2;

	users->USER1->NEXTINST = U1;  // Set NEXTINST

	users->user_id = 0;
	users->users_quantity = users_quantity;

	return 0;
}

void users_start(SharedState* shared_state) {
	// USER1 node represents action U1 and it is initially the sole entry in the WAIT list
	immed(shared_state, shared_state->users->USER1);
}

// [Enter, prepare for successor]
static void U1(SharedState* shared_state, ElevatorNode* C) {
	// User fabric

	Users* users = shared_state->users;

	// Not in MIX -- Python stops once its fixed user list is exhausted;
	// this port stops once users_quantity users have been generated
	if (users->user_id >= users->users_quantity) return;

	// 1 JMP VALUES
	Values user_values = values();

	// 3 Create User
	ElevatorNode* user = storage_pool_pop(users->storage_pool);
	if (user == NULL) return;

	user->IN = user_values.IN;
	user->OUT = user_values.OUT;
	user->GIVEUPTIME = user_values.GIVEUPTIME;
	user->id = users->user_id;

	// 4 increment user_id (not in Coroutine U)
	users->user_id += 1;

	// 2 LDA INTERTIME (time before another user enters) / JMP HOLD
	hold(shared_state, C, user_values.INTERTIME);

	trace(shared_state, "U1", "User %u arrives at floor %u, destination is %u, give up %u",
	            user->id, user->IN, user->OUT, shared_state->TIME + user->GIVEUPTIME);

	// 5 to U2, with C now the new node
	U2(shared_state, user);
}

// [Signal and wait]
static void U2(SharedState* shared_state, ElevatorNode* user) {
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
static void U3(SharedState* shared_state, ElevatorNode* user) {
	// Insert node at right end of QUEUE[IN]
	elevator_list_insert_node_at_rear(&shared_state->QUEUE[user->IN], user);

	U4A(shared_state, user);
}

// [Wait GIVEUPTIME units]
static void U4A(SharedState* shared_state, ElevatorNode* user) {
	// LDA GIVEUPTIME
	// JMP HOLDC
	uint32_t delay = user->GIVEUPTIME;
	holdc(shared_state, user, delay, U4);
}

// [Give up]
static void U4(SharedState* shared_state, ElevatorNode* user) {
	Elevator* elevator = shared_state->elevator;

	// If the user's IN floor differs from the elevator's current FLOOR: give up
	if (user->IN != elevator->FLOOR) {
		trace(shared_state, "U4", "User %u decides to give up, leaves the system", user->id);
		U6_impl(shared_state, user, false);
		return;
	}

	// JANZ U4A: doors still busy: reschedule U4
	if (elevator->D1 != 0) {
		U4A(shared_state, user);
		return;
	}

	trace(shared_state, "U4", "User %u decides to give up, leaves the system", user->id);
	U6_impl(shared_state, user, false);
}

// [Get in]
void U5(SharedState* shared_state, ElevatorNode* user) {
	Elevator* elevator = shared_state->elevator;

	trace(shared_state, "U5", "User %u gets in", user->id);

	// 1. This user now leaves QUEUE and enters ELEVATOR

	// Delete User from QUEUE[IN]
	elevator_list_delete_node(user);

	// Insert it at right of ELEVATOR
	elevator_list_insert_node_at_rear(&shared_state->ELEVATOR_LIST, user);

	// 2. Set CALLCAR[OUT] = 1
	shared_state->CALLS[user->OUT] |= CALLCAR;

	// 3. Jump if STATE != NEUTRAL
	if (elevator->STATE != 0) {
		// J5NZ CYCLE
		return;
	}

	// Knuth's style: set STATE = GOINGUP or GOINGDOWN as appropriate
	elevator->STATE = user->OUT - elevator->FLOOR;

	// 4. Remove action E5 from WAIT list
	elevator_list_delete_nodew(elevator->ELEV2);

	// Restart E5 after 25 units
	uint32_t delay = 25;
	E5A(shared_state, delay);
}

// [Get out]
void U6(SharedState* shared_state, ElevatorNode* user) {
	U6_impl(shared_state, user, true);
}

static void U6_impl(SharedState* shared_state, ElevatorNode* user, bool is_print) {
	if (is_print) {
		trace(shared_state, "U6", "User %u gets out, leaves the system", user->id);
	}

	// JMP DELETE
	elevator_list_delete_node(user);

	// Not in MIX -- Python relies on garbage collection here; C manages the
	// storage pool manually, so the node must be returned once the user leaves
	storage_pool_push(shared_state->users->storage_pool, user);
}
