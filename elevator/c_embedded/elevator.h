#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <stdint.h>
#include <stdbool.h>

#include "storage_pool.h"
#include "elevator_settings.h"

typedef struct Elevator {
    SharedState* shared_state;
    int32_t  STATE;
    uint32_t FLOOR;
    uint32_t D1;  			// a variable that is zero except during the time people are getting in or out of the elevator
    uint32_t D2;  			// a variable that becomes zero if the elevator has sat on one floor without moving for 30 sec or more
    uint32_t D3;  			// a variable that is zero except when the doors are open but nobody is getting in or out of the elevator

    ElevatorNode* ELEV1;   	// This node represents the elevator actions, except for E5 and E9
    ElevatorNode* ELEV2;   	// Represents the independent elevator action at E5
    ElevatorNode* ELEV3;   	// Represents the independent elevator action at E9

    bool     first_search; 	// Look at E3-E4
} Elevator;

uint32_t elevator_init(Elevator* elevator, SharedState* shared_state, Storage_Pool* storage_pool);
void E1(SharedState* shared_state, ElevatorNode* C);
void E3(SharedState* shared_state, ElevatorNode* C);
void E5A(SharedState* shared_state, uint32_t delay);
void E6(SharedState* shared_state, ElevatorNode* C);

#endif
