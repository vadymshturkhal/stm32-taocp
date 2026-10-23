// users.cpp - users.c ported 1:1 (Knuth's U1-U6)
#include <cstdint>
#include "cstyle.hpp"

namespace cstyle {

// Private functions
static void U1(SharedState* shared_state, ElevatorNode* C);
static void U2(SharedState* shared_state, ElevatorNode* user);
static void U3(SharedState* shared_state, ElevatorNode* user);
static void U4(SharedState* shared_state, ElevatorNode* user);
static void U4A(SharedState* shared_state, ElevatorNode* user);
static void U6_impl(SharedState* shared_state, ElevatorNode* user, bool is_print);

std::uint32_t users_init(Users* users, SharedState* shared_state, StoragePool* storage_pool, std::uint32_t users_quantity) {
    if (users == nullptr || shared_state == nullptr || storage_pool == nullptr) return 1;

    users->shared_state = shared_state;
    shared_state->users = users;
    users->storage_pool = storage_pool;

    users->USER1 = static_cast<ElevatorNode*>(storage_pool_pop(storage_pool));
    if (users->USER1 == nullptr) return 2;
    users->USER1->NEXTINST = U1;

    users->user_id = 0;
    users->users_quantity = users_quantity;
    return 0;
}

void users_start(SharedState* shared_state) {
    // USER1 represents action U1 and is initially the sole entry in the WAIT list
    immed(shared_state, shared_state->users->USER1);
}

// [Enter, prepare for successor]
static void U1(SharedState* shared_state, ElevatorNode* C) {
    Users* users = shared_state->users;

    // Not in MIX: stop once users_quantity users have been generated
    if (users->user_id >= users->users_quantity) return;

    const Values user_values = values();

    auto* user = static_cast<ElevatorNode*>(storage_pool_pop(users->storage_pool));
    if (user == nullptr) return;

    user->IN = user_values.IN;
    user->OUT = user_values.OUT;
    user->GIVEUPTIME = user_values.GIVEUPTIME;
    user->id = users->user_id;
    users->user_id += 1;

    hold(shared_state, C, user_values.INTERTIME);

    trace(shared_state, "U1", "User %u arrives at floor %u, destination is %u, give up %u",
          static_cast<unsigned>(user->id), static_cast<unsigned>(user->IN), static_cast<unsigned>(user->OUT),
          static_cast<unsigned>(shared_state->TIME + user->GIVEUPTIME));

    U2(shared_state, user);
}

// [Signal and wait]
static void U2(SharedState* shared_state, ElevatorNode* user) {
    Elevator* elevator = shared_state->elevator;

    if (elevator->FLOOR == user->IN) {
        // The doors are closing (next action E6): reposition the elevator at E3
        if (elevator->ELEV1->NEXTINST == E6) {
            elevator->ELEV1->NEXTINST = E3;
            elevator_list_delete_nodew(elevator->ELEV1);
            immed(shared_state, elevator->ELEV1);
            U3(shared_state, user);
            return;
        }

        // D3 != 0: start E4 again
        if (elevator->D3 != 0) {
            elevator->D1 = 1;
            elevator->D3 = 0;
            immed(shared_state, elevator->ELEV1);
            U3(shared_state, user);
            return;
        }
    }

    // In all other cases press CALLUP or CALLDOWN
    if (user->OUT > user->IN) {
        shared_state->CALLS[user->IN] |= CALLUP;
    } else {
        shared_state->CALLS[user->IN] |= CALLDOWN;
    }

    // If D2 == 0 or the elevator is dormant at E1: DECISION
    if (elevator->D2 == 0 || elevator->ELEV1->NEXTINST == E1) {
        decision(shared_state, U2);
    }

    U3(shared_state, user);
}

// [Enter queue]
static void U3(SharedState* shared_state, ElevatorNode* user) {
    elevator_list_insert_node_at_rear(&shared_state->QUEUE[user->IN], user);
    U4A(shared_state, user);
}

// [Wait GIVEUPTIME units]
static void U4A(SharedState* shared_state, ElevatorNode* user) {
    holdc(shared_state, user, user->GIVEUPTIME, U4);
}

// [Give up]
static void U4(SharedState* shared_state, ElevatorNode* user) {
    Elevator* elevator = shared_state->elevator;

    if (user->IN != elevator->FLOOR) {
        trace(shared_state, "U4", "User %u decides to give up, leaves the system", static_cast<unsigned>(user->id));
        U6_impl(shared_state, user, false);
        return;
    }

    // Doors still busy: wait again
    if (elevator->D1 != 0) {
        U4A(shared_state, user);
        return;
    }

    trace(shared_state, "U4", "User %u decides to give up, leaves the system", static_cast<unsigned>(user->id));
    U6_impl(shared_state, user, false);
}

// [Get in]
void U5(SharedState* shared_state, ElevatorNode* user) {
    Elevator* elevator = shared_state->elevator;

    trace(shared_state, "U5", "User %u gets in", static_cast<unsigned>(user->id));

    // Leave QUEUE, enter ELEVATOR
    elevator_list_delete_node(user);
    elevator_list_insert_node_at_rear(&shared_state->ELEVATOR_LIST, user);

    shared_state->CALLS[user->OUT] |= CALLCAR;

    if (elevator->STATE != 0) return;   // J5NZ CYCLE

    elevator->STATE = static_cast<std::int32_t>(user->OUT) - static_cast<std::int32_t>(elevator->FLOOR);

    // Remove E5 from the WAIT list, restart it after 25 units
    elevator_list_delete_nodew(elevator->ELEV2);
    E5A(shared_state, 25);
}

// [Get out]
void U6(SharedState* shared_state, ElevatorNode* user) {
    U6_impl(shared_state, user, true);
}

static void U6_impl(SharedState* shared_state, ElevatorNode* user, bool is_print) {
    if (is_print) {
        trace(shared_state, "U6", "User %u gets out, leaves the system", static_cast<unsigned>(user->id));
    }

    elevator_list_delete_node(user);
    storage_pool_push(shared_state->users->storage_pool, user);
}

}  // namespace cstyle
