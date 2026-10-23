// elevator.cpp - elevator.c ported 1:1 (Knuth's E1-E9)
#include <cstdint>
#include "cstyle.hpp"

namespace cstyle {

// Private functions
static void E1A(Elevator* elevator);
static void E2(SharedState* shared_state, ElevatorNode* C);
static void E2A(SharedState* shared_state, ElevatorNode* C, std::uint32_t delay);
static void E4(SharedState* shared_state, ElevatorNode* C);
static void E4A(SharedState* shared_state, std::uint32_t delay);
static void E5(SharedState* shared_state, ElevatorNode* C);
static void E6B(Elevator* elevator);
static void E7(SharedState* shared_state, ElevatorNode* C);
static void E7_continue(SharedState* shared_state, ElevatorNode* C);
static void E8(SharedState* shared_state, ElevatorNode* C);
static void E8_continue(SharedState* shared_state, ElevatorNode* C);
static void E9(SharedState* shared_state, ElevatorNode* C);

std::uint32_t elevator_init(Elevator* elevator, SharedState* shared_state, StoragePool* storage_pool) {
    if (elevator == nullptr || shared_state == nullptr || storage_pool == nullptr) return 1;

    elevator->shared_state = shared_state;
    shared_state->elevator = elevator;

    elevator->ELEV1 = static_cast<ElevatorNode*>(storage_pool_pop(storage_pool));
    if (elevator->ELEV1 == nullptr) return 2;
    elevator->ELEV2 = static_cast<ElevatorNode*>(storage_pool_pop(storage_pool));
    if (elevator->ELEV2 == nullptr) return 2;
    elevator->ELEV3 = static_cast<ElevatorNode*>(storage_pool_pop(storage_pool));
    if (elevator->ELEV3 == nullptr) return 2;

    elevator->ELEV1->NEXTINST = E1;
    elevator->ELEV2->NEXTINST = E5;
    elevator->ELEV3->NEXTINST = E9;

    elevator->STATE = 0;  // Neutral
    elevator->FLOOR = HOME_FLOOR;
    elevator->D1 = 0;
    elevator->D2 = 0;
    elevator->D3 = 0;
    elevator->first_search = false;

    // Matches MIX memory starting zero (LLINK of ELEV3, checked in E3, zeroed by E9)
    elevator->ELEV3->left1 = nullptr;
    return 0;
}

static void E1A(Elevator* elevator) {
    cycle1(elevator->ELEV1, E1);
}

// [Wait for call]
void E1(SharedState*, ElevatorNode*) {}

// [Change of state?]
static void E2(SharedState* shared_state, ElevatorNode* C) {
    Elevator* elevator = shared_state->elevator;

    trace(shared_state, "E2", "Elevator stops");

    if (elevator->STATE > 0) {
        // Are there calls for higher floors?
        bool has_higher_call = false;
        for (std::uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
            if (shared_state->CALLS[j] != 0) {
                has_higher_call = true;
                break;
            }
        }
        if (has_higher_call) {
            E3(shared_state, C);
            return;
        }

        // Have passengers in the elevator called for lower floors?
        bool has_lower_callcar = false;
        for (std::uint32_t j = 0; j < elevator->FLOOR; j++) {
            if ((shared_state->CALLS[j] & CALLCAR) != 0) {
                has_lower_callcar = true;
                break;
            }
        }
        elevator->STATE = has_lower_callcar ? -elevator->STATE : 0;
    } else if (elevator->STATE < 0) {
        // Are there calls for lower floors?
        bool has_lower_call = false;
        for (std::uint32_t j = 0; j < elevator->FLOOR; j++) {
            if (shared_state->CALLS[j] != 0) {
                has_lower_call = true;
                break;
            }
        }
        if (has_lower_call) {
            E3(shared_state, C);
            return;
        }

        // Have passengers in the elevator called for higher floors?
        bool has_higher_callcar = false;
        for (std::uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
            if ((shared_state->CALLS[j] & CALLCAR) != 0) {
                has_higher_callcar = true;
                break;
            }
        }
        elevator->STATE = has_higher_callcar ? -elevator->STATE : 0;
    } else {
        return;  // STATE == NEUTRAL: should not happen
    }

    shared_state->CALLS[elevator->FLOOR] = 0;
    E3(shared_state, C);
}

static void E2A(SharedState* shared_state, ElevatorNode* C, std::uint32_t delay) {
    holdc(shared_state, C, delay, E2);
}

// [Open door]
void E3(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    trace(shared_state, "E3", "Elevator doors start to open");

    // If activity E9 is already scheduled: remove it from the WAIT list
    if (elevator->ELEV3->left1 != nullptr) {
        elevator_list_delete_nodew(elevator->ELEV3);
    }

    hold(shared_state, elevator->ELEV3, 300);   // E9 after 300 units
    hold(shared_state, elevator->ELEV2, 76);    // E5 after 76 units

    elevator->D2 = 1;
    elevator->D1 = 1;
    elevator->first_search = true;

    E4A(shared_state, 20);
}

// [Let people out, in]
static void E4(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    bool first_search = elevator->first_search;
    elevator->first_search = false;

    // Search the ELEVATOR list from right to left
    ElevatorNode* node = shared_state->ELEVATOR_LIST.head->left2;
    while (node != shared_state->ELEVATOR_LIST.head) {
        if (node->OUT != elevator->FLOOR) {
            node = node->left2;
            continue;
        }
        // Send the user to U6, at the front of the WAIT list
        node->NEXTINST = U6;
        immed(shared_state, node);
        E4A(shared_state, 25);
        return;
    }

    node = shared_state->QUEUE[elevator->FLOOR].head->right2;
    if (node != shared_state->QUEUE[elevator->FLOOR].head) {
        // Cancel U4 for this user (DELETEW), replace it by U5
        elevator_list_delete_nodew(node);
        node->NEXTINST = U5;
        immed(shared_state, node);
        E4A(shared_state, 25);
        return;
    }

    // QUEUE is empty
    elevator->D1 = 0;
    elevator->D3 = 1;

    if (first_search) {
        trace(shared_state, "E4", "Doors open, nobody is there");
    }
}

static void E4A(SharedState* shared_state, std::uint32_t delay) {
    holdc(shared_state, shared_state->elevator->ELEV1, delay, E4);
}

void E5A(SharedState* shared_state, std::uint32_t delay) {
    holdc(shared_state, shared_state->elevator->ELEV2, delay, E5);
}

// [Close door]
static void E5(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    if (elevator->D1 != 0) {
        trace(shared_state, "E5", "Elevator doors flutter");
        E5A(shared_state, 40);
        return;
    }

    trace(shared_state, "E5", "Elevator doors start to close");
    elevator->D3 = 0;
    holdc(shared_state, elevator->ELEV1, 20, E6);
}

// [Prepare to move]
void E6(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    if (elevator->STATE >= 0) {
        shared_state->CALLS[elevator->FLOOR] &= ~(CALLUP | CALLCAR);
    }
    if (elevator->STATE <= 0) {
        shared_state->CALLS[elevator->FLOOR] &= ~(CALLCAR | CALLDOWN);
    }
    if (elevator->STATE == 0) {
        decision(shared_state, E6);
    }

    E6B(elevator);
}

static void E6B(Elevator* elevator) {
    if (elevator->STATE == 0) {
        E1A(elevator);
        return;
    }

    if (elevator->D2 != 0) {
        // Cancel activity E9 (STZ 0,6)
        elevator_list_delete_nodew(elevator->ELEV3);
        elevator->ELEV3->left1 = nullptr;
    }

    if (elevator->STATE < 0) {
        holdc(elevator->shared_state, elevator->ELEV1, 15, E8);
        return;
    }
    holdc(elevator->shared_state, elevator->ELEV1, 15, E7);
}

// [Go up a floor]
static void E7(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    trace(shared_state, "E7", "Elevator moving up");

    elevator->FLOOR += 1;
    holdc(shared_state, elevator->ELEV1, 51, E7_continue);
}

static void E7_continue(SharedState* shared_state, ElevatorNode* C) {
    Elevator* elevator = shared_state->elevator;

    if ((shared_state->CALLS[elevator->FLOOR] & (CALLCAR | CALLUP)) != 0) {
        E2A(shared_state, elevator->ELEV1, 14);
        return;
    }

    if (elevator->FLOOR == HOME_FLOOR || (shared_state->CALLS[elevator->FLOOR] & CALLDOWN) != 0) {
        bool has_higher_call = false;
        for (std::uint32_t j = elevator->FLOOR + 1; j < FLOORS; j++) {
            if (shared_state->CALLS[j] != 0) {
                has_higher_call = true;
                break;
            }
        }
        if (has_higher_call) {
            E7(shared_state, C);
            return;
        }
    } else {
        E7(shared_state, C);
        return;
    }

    E2A(shared_state, elevator->ELEV1, 14);
}

// [Go down a floor]
static void E8(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    trace(shared_state, "E8", "Elevator moving down");

    elevator->FLOOR -= 1;
    holdc(shared_state, elevator->ELEV1, 61, E8_continue);
}

static void E8_continue(SharedState* shared_state, ElevatorNode* C) {
    Elevator* elevator = shared_state->elevator;

    if ((shared_state->CALLS[elevator->FLOOR] & (CALLCAR | CALLDOWN)) != 0) {
        E2A(shared_state, elevator->ELEV1, 23);
        return;
    }

    if (elevator->FLOOR == HOME_FLOOR || (shared_state->CALLS[elevator->FLOOR] & CALLUP) != 0) {
        bool has_lower_call = false;
        for (std::uint32_t j = 0; j < elevator->FLOOR; j++) {
            if (shared_state->CALLS[j] != 0) {
                has_lower_call = true;
                break;
            }
        }
        if (has_lower_call) {
            E8(shared_state, C);
            return;
        }
    } else {
        E8(shared_state, C);
        return;
    }

    E2A(shared_state, elevator->ELEV1, 23);
}

// [Set inaction indicator]
static void E9(SharedState* shared_state, ElevatorNode*) {
    Elevator* elevator = shared_state->elevator;

    // STZ 0,6: ELEV3 is no longer scheduled (checked by E3)
    elevator->ELEV3->left1 = nullptr;
    elevator->D2 = 0;
    decision(shared_state, E9);
}

}  // namespace cstyle
