#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_stack.h"

bool c_stack_push_tub(Stack* stack, uint32_t info) {
	// Translation Unit Boundary case
	// return false if Overflow, else true

	// 1
	if (stack->avail == NULL) return false;	// Overflow

	Node* P = stack->avail;
	stack->avail = stack->avail->link;

	// 2
	P->info = info;

	// 3
	P->link = stack->top;

	// 4
	stack->top = P;

	return true;
}

uint32_t c_stack_pop_tub(Stack* stack, bool* pop_is_success) {
	// Translation Unit Boundary case
	// using pop_is_success flag for Underflow checking

	// 1
	if (stack->top == NULL) {
		*pop_is_success = false;	// Underflow
		return 0;
	}

	Node* P = stack->top;

	// 2
	stack->top = P->link;

	// 3
	uint32_t info = P->info;

	// 4
	P->link = stack->avail;
	stack->avail = P;

	return info;
}


Node* init_storage_pool(Stack* stack, uint32_t size) {
	Node* avail = (Node*)(stack + 1);
	Node* tmp;

	avail->info = size;
	avail->link = NULL;
	size--;

	while (size > 0) {
		tmp = avail+1;
		tmp->info = size;
		tmp->link = avail;

		avail = tmp;
		size--;
	}

	return avail;
}

Stack* c_create_stack(void* memory, uint32_t size) {
	// size must be greater than 0

	Stack* stack = (Stack*)memory;
	stack->top = NULL;

	Node* avail = init_storage_pool(stack, size);
	stack->avail = avail;

	return stack;
}
