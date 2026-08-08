#ifndef C_ELEVATOR_SETTINGS_H
#define C_ELEVATOR_SETTINGS_H

#include <stdint.h>

typedef void (*NextInst)(struct ElevatorNode*);

// Without INTERTIME and USER_NAME
typedef struct ElevatorNode {
    struct ElevatorNode* left1;  // WAIT list
    struct ElevatorNode* right1;
    struct ElevatorNode* left2;  // QUEUE, ELEVATOR_LIST
    struct ElevatorNode* right2;

    uint32_t nexttime;
    NextInst nextinst;

    uint32_t in;                 // UserInfo.IN
    uint32_t out;                // UserInfo.OUT
    uint32_t giveuptime;         // UserInfo.GIVEUPTIME
} DoublyNode;


#endif
