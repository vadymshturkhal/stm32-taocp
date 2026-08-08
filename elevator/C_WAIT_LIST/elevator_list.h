#ifndef ELEVATOR_LIST_H
#define ELEVATOR_LIST_H

#include <stdint.h>

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
//void insert_node_at_frontw(ElevatorList* elevator_list, ElevatorNode* node);
//void insert_node_at_rear(ElevatorList* elevator_list, ElevatorNode* node);
//void delete_nodew(ElevatorList* elevator_list, ElevatorNode* node);
//void delete_node(ElevatorList* elevator_list, ElevatorNode* node);

#endif
