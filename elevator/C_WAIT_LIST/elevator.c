#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator.h"
#include "users.h"

// Private function
static void E6B(Elevator* elevator);
static void E7_continue(SharedState* shared_state, ElevatorNode* C);

uint32_t elevator_init(Elevator* elevator, SharedState* shared_state, Storage_Pool* storage_pool) {
	if (elevator == NULL || shared_state == NULL || storage_pool == NULL) return 1;

	elevator->shared_state = shared_state;
	shared_state->elevator = elevator;   // add Elevator to shared state

	elevator->ELEV1 = storage_pool_pop(storage_pool);
	if (elevator->ELEV1 == NULL) return 2;

	elevator->ELEV2 = storage_pool_pop(storage_pool);
	if (elevator->ELEV2 == NULL) return 2;

	elevator->ELEV3 = storage_pool_pop(storage_pool);
	if (elevator->ELEV3 == NULL) return 2;

	elevator->ELEV1->NEXTINST = E1;
	elevator->ELEV2->NEXTINST = E5;
	elevator->ELEV3->NEXTINST = E9;

	elevator->STATE = 0;  // Neutral
	elevator->FLOOR = HOME_FLOOR;

	elevator->D1 = 0;
	elevator->D2 = 0;
	elevator->D3 = 0;

	elevator->first_search = false;

	// Matches MIX memory starting zero (LLINK of ELEV3, checked in E3, zeroed by E9)
	elevator->ELEV3->left1 = NULL;

	return 0;
}

void E1A(Elevator* elevator) {
	// Set NEXTINST = E1 and go to CYCLE
    cycle1(elevator->ELEV1, E1);
}

// [Wait for call]
void E1(SharedState* shared_state, ElevatorNode* C) {

}

// [Change of state?]
void E2(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// TODO: trace print (Table 1 style) -- no logging yet

	// 1 STATE is GOINGUP
	if (elevator->STATE > 0) {
		// Are there calls for higher floors?
		bool has_higher_call = false;
		for (uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
			if (shared_state->CALLS[j] != 0) {
				has_higher_call = true;
				break;
			}
		}

		// If yes, go to E3
		if (has_higher_call) {
			E3(shared_state, C);
			return;
		}

		// Have passengers in the elevator called for lower floors?
		bool has_lower_callcar = false;
		for (uint32_t j = 0; j < elevator->FLOOR; j++) {
			if (shared_state->CALLS[j] & CALLCAR) {
				has_lower_callcar = true;
				break;
			}
		}

		// If yes, reverse direction of STATE, else set STATE to NEUTRAL
		if (has_lower_callcar) {
			elevator->STATE = -elevator->STATE;
		} else {
			elevator->STATE = 0;
		}
	}

	// 2 STATE is GOINGDOWN
	else if (elevator->STATE < 0) {
		// Are there calls for lower floors?
		bool has_lower_call = false;
		for (uint32_t j = 0; j < elevator->FLOOR; j++) {
			if (shared_state->CALLS[j] != 0) {
				has_lower_call = true;
				break;
			}
		}

		// If yes, go to E3
		if (has_lower_call) {
			E3(shared_state, C);
			return;
		}

		// Have passengers in the elevator called for higher floors?
		bool has_higher_callcar = false;
		for (uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
			if (shared_state->CALLS[j] & CALLCAR) {
				has_higher_callcar = true;
				break;
			}
		}

		// If yes, reverse direction of STATE, else set STATE to NEUTRAL
		if (has_higher_callcar) {
			elevator->STATE = -elevator->STATE;
		} else {
			elevator->STATE = 0;
		}
	}
	else {
		// STATE == NEUTRAL: should not happen, caller invariant
		return;
	}

	// Set all CALL variables for the current FLOOR to zero
	shared_state->CALLS[elevator->FLOOR] = 0;

	// Jump to E3
	E3(shared_state, C);
}

void E2A(SharedState* shared_state, ElevatorNode* C, uint32_t delay) {
	// JMP HOLDC
	holdc(shared_state, C, delay, E2);
}

// [Open door]
void E3(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// TODO: trace print (Table 1 style) -- no logging yet

	// If activity already scheduled: remove it from WAIT list
	// (Knuth: LDA 0,6 / JANZ DELETEW -- ELEV3->left1 is that same cell,
	// zeroed by E9 when it fires uncancelled, and zeroed once at elevator_init)
	if (elevator->ELEV3->left1 != NULL) {
		// DELETEW
		elevator_list_delete_nodew(elevator->ELEV3);
	}

	// Schedule activity E9 after 300 units
	uint32_t delay = 300;
	hold(shared_state, elevator->ELEV3, delay);

	// Schedule activity E5 after 76 units
	delay = 76;
	hold(shared_state, elevator->ELEV2, delay);

	// Set D2 to nonzero
	elevator->D2 = 1;

	// Set D1 to nonzero
	elevator->D1 = 1;

	// Printing
//	elevator->first_search = true;

	delay = 20;
	E4A(shared_state, delay);
}

// [Let people out, in]
void E4(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// Printing
//	bool first_search = elevator->first_search;
//	elevator->first_search = false;

	// node = LOC(ELEVATOR)
	ElevatorNode* node = shared_state->ELEVATOR_LIST.head;
	node = node->left2;

	// Search ELEVATOR list from left to right
	while (node != shared_state->ELEVATOR_LIST.head) {
		// Compare OUT(node) with FLOOR

		// If not equal: continue
		if (node->OUT != elevator->FLOOR) {
			node = node->left2;
			continue;
		}

		// Otherwise prepare to send User to U6:
		// Set NEXTINST(node)
		node->NEXTINST = U6;

		// Put user at front of the WAIT list
		immed(shared_state, node);

		// Wait 25 units and repeat E4A
		uint32_t delay = 25;
		E4A(shared_state, delay);

		// Return to simulate other events
		return;
	}

	// If node == ELEVATOR_LIST.head: search is complete

	node = shared_state->QUEUE[elevator->FLOOR].head;
	node = node->right2;

	if (node != shared_state->QUEUE[elevator->FLOOR].head) {
		// Cancel action U4 for this User
		// DELETEW
		elevator_list_delete_nodew(node);

		// Prepare to replace U4 by U5
		// Set NEXTINST(node)
		node->NEXTINST = U5;

		// Put User at front of the WAIT list
		immed(shared_state, node);

		// Wait 25 units and repeat E4
		uint32_t delay = 25;
		E4A(shared_state, delay);

		// Return to simulate other events
		return;
	}

	// If node == QUEUE[FLOOR].head: QUEUE is empty

	// Set D1 = 0
	elevator->D1 = 0;

	// Set D3 nonzero
	elevator->D3 = 1;

	// TODO: trace print (Table 1 style) -- no logging yet
//	if (first_search) {
//
//	}
}

void E4A(SharedState* shared_state, uint32_t delay) {
	// JMP HOLDC
	holdc(shared_state, shared_state->elevator->ELEV1, delay, E4);
}

void E5A(SharedState* shared_state, uint32_t delay) {
	// JMP HOLDC
	holdc(shared_state, shared_state->elevator->ELEV2, delay, E5);
}

// [Close door]
void E5(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// Is D1 == 0?
	// If not, people are getting in or out
	if (elevator->D1 != 0) {
		// TODO: trace print (Table 1 style) -- "Elevator doors flutter"

		// Wait 40 units, repeat E5
		uint32_t delay = 40;
		E5A(shared_state, delay);
		return;
	}

	// TODO: trace print (Table 1 style) -- "Elevator doors start to close"

	// If D1 == 0: set D3 = 0
	elevator->D3 = 0;

	// Wait 20 units, then go to E6
	uint32_t delay = 20;
	holdc(shared_state, elevator->ELEV1, delay, E6);
}

// [Prepare to move]
void E6(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// If STATE != GOINGDOWN: CALLUP and CALLCAR on this floor are reset
	if (elevator->STATE >= 0) {
		shared_state->CALLS[elevator->FLOOR] &= ~(CALLUP | CALLCAR);
	}

	// If STATE != GOINGUP: reset CALLCAR and CALLDOWN
	if (elevator->STATE <= 0) {
		shared_state->CALLS[elevator->FLOOR] &= ~(CALLCAR | CALLDOWN);
	}

	// Perform DECISION subroutine
	if (elevator->STATE == 0) {
		// J5Z DECISION
		decision(shared_state, E6);
	}

	E6B(elevator);
}

static void E6B(Elevator* elevator) {
	// If STATE == NEUTRAL: go to E1 and wait
	if (elevator->STATE == 0) {
		E1A(elevator);  // NOTE: can skip this line
		return;
	}

	if (elevator->D2 != 0) {
		// Cancel activity E9
		elevator_list_delete_nodew(elevator->ELEV3);
	}

	// Wait 15 units of time
	uint32_t delay = 15;

	// If STATE == GOINGDOWN, go to E8
	if (elevator->STATE < 0) {
		holdc(elevator->shared_state, elevator->ELEV1, delay, E8);
		return;
	}

	// Else go to E7
	holdc(elevator->shared_state, elevator->ELEV1, delay, E7);
}

// [Go up a floor]
void E7(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// TODO: trace print (Table 1 style) -- "Elevator moving up"

	// INC4 1
	elevator->FLOOR += 1;

	// Wait 51 units
	uint32_t delay = 51;
	holdc(shared_state, elevator->ELEV1, delay, E7_continue);
}

// Not in MIX
static void E7_continue(SharedState* shared_state, ElevatorNode* C) {
	Elevator* elevator = shared_state->elevator;

	// Is CALLCAR[FLOOR] or CALLUP[FLOOR] != 0?
	bool is_callcar_or_callup = shared_state->CALLS[elevator->FLOOR] & (CALLCAR | CALLUP);

	// If yes: it is time to stop elevator
	if (is_callcar_or_callup) {
		// Wait 14 units and go to E2
		uint32_t delay = 14;
		E2A(shared_state, elevator->ELEV1, delay);
		return;
	}

	// If not
	// Is FLOOR == HOME_FLOOR or CALLDOWN[FLOOR] != 0?
	if (elevator->FLOOR == HOME_FLOOR || (shared_state->CALLS[elevator->FLOOR] & CALLDOWN)) {
		// Are there calls for higher floors?
		bool has_higher_call = false;
		for (uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
			if (shared_state->CALLS[j] != 0) {
				has_higher_call = true;
				break;
			}
		}

		// If yes: repeat E7
		if (has_higher_call) {
			E7(shared_state, C);
			return;
		}
	} else {
		// repeat E7
		E7(shared_state, C);
		return;
	}

	// It's time to stop elevator
	uint32_t delay = 14;
	E2A(shared_state, elevator->ELEV1, delay);
}

// [Go down a floor]
void E8(SharedState* shared_state, ElevatorNode* C) {

}

// [Set inaction indicator]
void E9(SharedState* shared_state, ElevatorNode* C) {
	// Set D2 = 0
	shared_state->elevator->D2 = 0;
	decision(shared_state, E9);
}
