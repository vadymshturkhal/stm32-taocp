#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_settings.h"
#include "elevator.h"

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
	elevator->first_search = true;

	delay = 20;
	E4A(shared_state, delay);
}

// [Let people out, in]
void E4(SharedState* shared_state, ElevatorNode* C) {

}

void E4A(SharedState* shared_state, uint32_t delay) {
	// JMP HOLDC
	holdc(shared_state, shared_state->elevator->ELEV1, delay, E4);
}

void E6(SharedState* shared_state, ElevatorNode* C) {

}
