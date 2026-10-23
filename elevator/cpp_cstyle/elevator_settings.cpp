// elevator_settings.cpp - elevator_settings.c ported 1:1
#include <cstdint>
#include "cstyle.hpp"

namespace cstyle {

std::uint32_t shared_state_init(SharedState* shared_state, StoragePool* storage_pool) {
    if (shared_state == nullptr || storage_pool == nullptr) return 1;

    shared_state->TIME = 0;
    for (std::uint32_t i = 0; i < FLOORS; i++) {
        shared_state->CALLS[i] = 0;
    }

    std::uint32_t status = elevator_list_init(&shared_state->ELEVATOR_LIST, storage_pool);
    if (status != 0) return status;

    for (std::uint32_t i = 0; i < FLOORS; i++) {
        status = elevator_list_init(&shared_state->QUEUE[i], storage_pool);
        if (status != 0) return status;
    }

    status = elevator_list_init(&shared_state->WAIT_LIST, storage_pool);
    if (status != 0) return status;

    shared_state->WAIT_LIST.head->NEXTTIME = 0;  // Knuth's sentinel

    shared_state->elevator = nullptr;
    return 0;
}

void immed(SharedState* shared_state, ElevatorNode* wait_node) {
    // Insert wait_node first in WAIT list
    wait_node->NEXTTIME = shared_state->TIME;
    elevator_list_insert_node_at_frontw(&shared_state->WAIT_LIST, wait_node);
}

void sortin(SharedState* shared_state, ElevatorNode* C) {
    // Sort node into WAIT list using LLINK1 and RLINK1
    ElevatorList* wait_list = &shared_state->WAIT_LIST;
    ElevatorNode* P = wait_list->head->left1;  // Last node

    // Compare NEXTTIME fields right to left (unsigned: the head's 0 stops it)
    while (C->NEXTTIME < P->NEXTTIME) {
        P = P->left1;
    }

    ElevatorNode* Q = P->right1;
    C->right1 = Q;
    C->left1 = P;
    P->right1 = C;
    Q->left1 = C;
}

void hold(SharedState* shared_state, ElevatorNode* node, std::uint32_t delay) {
    // Schedule node to run delay units from now, leaving NEXTINST alone
    node->NEXTTIME = shared_state->TIME + delay;
    sortin(shared_state, node);
}

void holdc(SharedState* shared_state, ElevatorNode* node, std::uint32_t delay, NextInst next_inst) {
    node->NEXTINST = next_inst;
    hold(shared_state, node, delay);
}

void cycle1(ElevatorNode* node, NextInst next_inst) {
    node->NEXTINST = next_inst;
}

void decision(SharedState* shared_state, NextInst caller) {
    Elevator* elevator = shared_state->elevator;

    // D1. Decision necessary?
    if (elevator->STATE != 0) return;

    // D2. Should door open?
    if (elevator->ELEV1->NEXTINST == E1 && shared_state->CALLS[HOME_FLOOR] != 0) {
        elevator->ELEV1->NEXTINST = E3;
        hold(shared_state, elevator->ELEV1, 20);
        return;
    }

    // D3. Any calls?
    std::int32_t j = -1;
    for (std::uint32_t i = 0; i < FLOORS; i++) {
        if (i == elevator->FLOOR) continue;
        if (shared_state->CALLS[i] == 0) continue;
        j = static_cast<std::int32_t>(i);
        break;
    }

    // All CALL[j], j != FLOOR, are zero
    if (j == -1) {
        // Is the caller E6 (MIX: E6B)?
        if (caller == E6) {
            j = static_cast<std::int32_t>(HOME_FLOOR);
        } else {
            return;
        }
    }

    // D4. Set STATE = j - FLOOR
    elevator->STATE = j - static_cast<std::int32_t>(elevator->FLOOR);

    // D5. Elevator dormant? Jump if not at E1 or j == HOME_FLOOR
    if (elevator->ELEV1->NEXTINST != E1 || elevator->STATE == 0) return;

    elevator->ELEV1->NEXTINST = E6;
    hold(shared_state, elevator->ELEV1, 20);
}

}  // namespace cstyle
