#ifndef C_ELEVATOR_SETTINGS_H
#define C_ELEVATOR_SETTINGS_H

#include <stdint.h>

#include "storage_pool.h"
#include "elevator_list.h"

#define FLOORS 5
#define HOME_FLOOR 2

#define CALLUP   0b100
#define CALLDOWN 0b010
#define CALLCAR  0b001

struct ElevatorNode;  // forward declaration so NextInst's tag has file scope

// Forward declaration only, this avoids elevator_settings.h <-> elevator.h including each other
typedef struct Elevator Elevator;
typedef struct Users Users;

typedef struct SharedState {
    uint32_t TIME;
    uint32_t CALLS[FLOORS];
    ElevatorList ELEVATOR_LIST;   // Users currently riding the car
    ElevatorList QUEUE[FLOORS];   // Users waiting at each floor
    ElevatorList WAIT_LIST;
    Elevator* elevator;
    Users* users;
} SharedState;

uint32_t shared_state_init(SharedState* shared_state, Storage_Pool* storage_pool);
void immed(SharedState* shared_state, ElevatorNode* wait_node);
void sortin(SharedState* shared_state, ElevatorNode* C);
void hold(SharedState* shared_state, ElevatorNode* node, uint32_t delay);
void holdc(SharedState* shared_state, ElevatorNode* node, uint32_t delay, NextInst next_inst);
void cycle1(ElevatorNode* node, NextInst next_inst);
void decision(SharedState* shared_state, NextInst caller);
void cycle(SharedState* shared_state);
//insert = elevator_list_insert_node_at_rear
//deletew = elevator_list_delete_nodew
//delete = elevator_list_delete_node

#endif
