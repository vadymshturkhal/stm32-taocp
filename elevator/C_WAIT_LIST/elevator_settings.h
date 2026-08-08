#ifndef C_ELEVATOR_SETTINGS_H
#define C_ELEVATOR_SETTINGS_H

#include <stdint.h>

#include "storage_pool.h"

#define FLOORS 5

#define CALLUP   0b100
#define CALLDOWN 0b010
#define CALLCAR  0b001

typedef void (*NextInst)(struct ElevatorNode*);

// NOTE: Without INTERTIME and USER_NAME
typedef struct ElevatorNode {
    struct ElevatorNode* left1;  // WAIT list
    struct ElevatorNode* right1;
    struct ElevatorNode* left2;  // QUEUE, ELEVATOR_LIST
    struct ElevatorNode* right2;
    uint32_t nexttime;
    NextInst nextinst;
    uint32_t in;                 // User
    uint32_t out;                // User
    uint32_t giveuptime;         // User
} ElevatorNode;

typedef struct {
	ElevatorNode* head;
	Storage_Pool* storage_pool;
	uint32_t size;
} ElevatorList;

// NOTE: Add Elevator and Users
typedef struct {
    uint32_t time;
    uint32_t calls[FLOORS];
    ElevatorList elevator_list;
    ElevatorList queue[FLOORS];
    ElevatorList wait_list;
} SharedState;

uint32_t elevator_list_init(ElevatorList* elevator_list, Storage_Pool* storage_pool);

#endif
