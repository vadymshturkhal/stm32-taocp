#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_stack.h"

Node* init_storage_pool(Stack* stack, uint32_t size) {
	if (size == 0) return NULL;

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
	Stack* stack = (Stack*)memory;
	stack->top = NULL;

	Node* avail = init_storage_pool(stack, size);
	stack->avail = avail;

	return stack;
}
