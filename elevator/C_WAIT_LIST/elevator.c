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

void E1(Elevator* elevator, ElevatorNode* C) {

}

void E2(Elevator* elevator, ElevatorNode* C) {

}

void E6(Elevator* elevator, ElevatorNode* C) {

}
