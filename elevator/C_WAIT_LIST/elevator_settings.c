#include <stdint.h>
#include <stddef.h>

#include "storage_pool.h"
#include "elevator_list.h"
#include "elevator_settings.h"

uint32_t shared_state_init(SharedState* shared_state, Storage_Pool* storage_pool) {
	if (shared_state == NULL || storage_pool == NULL) return 1;

	shared_state->TIME = 0;
	for (uint32_t i = 0; i < FLOORS; i++) {
		shared_state->CALLS[i] = 0;
	}

	uint32_t status;

	status = elevator_list_init(&shared_state->ELEVATOR_LIST, storage_pool);
	if (status != 0) return status;

	for (uint32_t i = 0; i < FLOORS; i++) {
		status = elevator_list_init(&shared_state->QUEUE[i], storage_pool);
		if (status != 0) return status;
	}

	status = elevator_list_init(&shared_state->WAIT_LIST, storage_pool);
	if (status != 0) return status;

	shared_state->WAIT_LIST.head->NEXTTIME = 0;  // set NEXTTIME of head node to 0

	shared_state->elevator = NULL;

	return 0;
}

void immed(SharedState* shared_state, ElevatorNode* wait_node) {
	// Insert wait_node first in WAIT list

	// 1 Set NEXTTIME(wait_node) = TIME
	wait_node->NEXTTIME = shared_state->TIME;

	// 2 Insert wait_node
	elevator_list_insert_node_at_frontw(&shared_state->WAIT_LIST, wait_node);
}

void sortin(SharedState* shared_state, ElevatorNode* C) {
    // Sort node into WAIT list using LLINK1 and RLINK1

	ElevatorList* wait_list = &shared_state->WAIT_LIST;
	ElevatorNode* P = wait_list->head->left1;  // Last node

	// Compare NEXTTIME fields right to left
	while (C->NEXTTIME < P->NEXTTIME) {
		P = P->left1;
	}

	// Insert to WAIT list
	ElevatorNode* Q = P->right1;
	C->right1 = Q;
	C->left1 = P;
	P->right1 = C;
	Q->left1 = C;
}

void hold(SharedState* shared_state, ElevatorNode* node, uint32_t delay) {
    // Schedule node to run delay units from now, leaving NEXTINST alone
    // U1 needs this variant: USER1 must keep resuming at U1 forever, so its
    // NEXTINST has to survive being rescheduled.

	node->NEXTTIME = shared_state->TIME + delay;
	sortin(shared_state, node);
}

void holdc(SharedState* shared_state, ElevatorNode* node, uint32_t delay, NextInst next_inst) {
	// Set NEXTINST and hold
	node->NEXTINST = next_inst;
	hold(shared_state, node, delay);
}

void cycle1(ElevatorNode* node, NextInst next_inst) {
	// Set NEXTINST of node
	node->NEXTINST = next_inst;
}
