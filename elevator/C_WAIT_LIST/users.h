#ifndef USERS_H
#define USERS_H

#include <stdint.h>

#include "storage_pool.h"
#include "elevator_list.h"
#include "elevator_settings.h"

typedef struct Users {
    SharedState* shared_state;
    ElevatorNode* USER1;
    uint32_t user_id;
} Users;

uint32_t users_init(Users* users, SharedState* shared_state, Storage_Pool* storage_pool);
void U1(SharedState* shared_state, ElevatorNode* C);
void U5(SharedState* shared_state, ElevatorNode* C);
void U6(SharedState* shared_state, ElevatorNode* C);

#endif
