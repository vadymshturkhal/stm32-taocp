#ifndef USERS_H
#define USERS_H

#include <stdint.h>

#include "storage_pool.h"
#include "elevator_list.h"
#include "elevator_settings.h"

typedef struct Users {
    SharedState* shared_state;
    Storage_Pool* storage_pool;   // used by U1 to allocate arriving users' nodes
    ElevatorNode* USER1;
    uint32_t user_id;
    uint32_t users_quantity;      // total users to generate before U1 stops for good
} Users;

uint32_t users_init(Users* users, SharedState* shared_state, Storage_Pool* storage_pool, uint32_t users_quantity);
void users_start(SharedState* shared_state);
void U5(SharedState* shared_state, ElevatorNode* C);
void U6(SharedState* shared_state, ElevatorNode* C);

#endif
