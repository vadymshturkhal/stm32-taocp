#ifndef ELEVATOR_LIST_H
#define ELEVATOR_LIST_H

#include <stdint.h>

#include "storage_pool.h"

struct ElevatorNode;  // forward declaration so NextInst's tag has file scope

typedef void (*NextInst)(struct ElevatorNode*);

// FIXME: ADD INTERTIME and USER_NAME
typedef struct ElevatorNode {
    struct ElevatorNode* left1;  // WAIT list
    struct ElevatorNode* right1;
    struct ElevatorNode* left2;  // QUEUE, ELEVATOR_LIST
    struct ElevatorNode* right2;

    uint32_t NEXTTIME;
    NextInst NEXTINST;

    uint32_t IN;                 // User
    uint32_t OUT;                // User
    uint32_t GIVEUPTIME;         // User
} ElevatorNode;

typedef struct {
	ElevatorNode* head;
	Storage_Pool* storage_pool;
} ElevatorList;

uint32_t elevator_list_init(ElevatorList* elevator_list, Storage_Pool* storage_pool);
void elevator_list_insert_node_at_frontw(ElevatorList* elevator_list, ElevatorNode* node);
void elevator_list_insert_node_at_rear(ElevatorList* elevator_list, ElevatorNode* node);
void elevator_list_delete_nodew(ElevatorList* elevator_list, ElevatorNode* node);
void elevator_list_delete_node(ElevatorList* elevator_list, ElevatorNode* node);

#endif
