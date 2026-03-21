// c_stack.h
#ifndef C_STACK_H	// prevent duplicate definitions
#define C_STACK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Node {
    uint32_t info;
    struct Node* link;
} Node;

typedef struct {
    uint32_t size;
    Node* top;
    Node* avail;
} Stack;

// Cold Path init
Stack* c_create_stack(void* memory, uint32_t size);

// Hot Path inline execution
static inline bool c_stack_push(Stack* stack, uint32_t info) {
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

static inline uint32_t c_stack_pop(Stack* stack, bool* pop_is_success) {
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

#endif // C_STACK_H
