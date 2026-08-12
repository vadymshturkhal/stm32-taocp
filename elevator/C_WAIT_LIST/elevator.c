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

	return 0;
}

void E1A(Elevator* elevator) {
	// Set NEXTINST = E1 and go to CYCLE
    cycle1(elevator->ELEV1, E1);
}

// [Wait for call]
void E1(Elevator* elevator, ElevatorNode* C) {

}

// [Change of state?]
void E2(Elevator* elevator, ElevatorNode* C) {
	// TODO: trace print (Table 1 style) -- no logging yet

	// 1 STATE is GOINGUP
	if (elevator->STATE > 0) {
		// Are there calls for higher floors?
		bool has_higher_call = false;
		for (uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
			if (elevator->shared_state->CALLS[j] != 0) {
				has_higher_call = true;
				break;
			}
		}

		// If yes, go to E3
		if (has_higher_call) {
			E3(elevator, C);
			return;
		}

		// Have passengers in the elevator called for lower floors?
		bool has_lower_callcar = false;
		for (uint32_t j = 0; j < elevator->FLOOR; j++) {
			if (elevator->shared_state->CALLS[j] & CALLCAR) {
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
			if (elevator->shared_state->CALLS[j] != 0) {
				has_lower_call = true;
				break;
			}
		}

		// If yes, go to E3
		if (has_lower_call) {
			E3(elevator, C);
			return;
		}

		// Have passengers in the elevator called for higher floors?
		bool has_higher_callcar = false;
		for (uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
			if (elevator->shared_state->CALLS[j] & CALLCAR) {
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
	elevator->shared_state->CALLS[elevator->FLOOR] = 0;

	// Jump to E3
	E3(elevator, C);
}

void E2A(Elevator* elevator, ElevatorNode* C, uint32_t delay) {
	// JMP HOLDC
	holdc(elevator->shared_state, C, delay, E2);
}

// [Open door]
void E3(Elevator* elevator, ElevatorNode* C) {

}

// [Let people out, in]
void E4(Elevator* elevator, ElevatorNode* C) {

}

void E4A(uint32_t delay) {

}

void E6(Elevator* elevator, ElevatorNode* C) {

}
