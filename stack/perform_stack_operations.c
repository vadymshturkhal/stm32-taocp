#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_stack.h"


uint8_t perform_c_stack_operations(uint16_t max_nodes, uint8_t operations_factor){
	// node info is uint32_t

	void* c_stack_memory = malloc(max_nodes * sizeof(Node) + sizeof(Stack));
	if (c_stack_memory == NULL) return 0;

	// slow
	//void* c_stack_memory[max_nodes * sizeof(Node) + sizeof(Stack)];


	Stack* stack = c_create_stack(c_stack_memory, max_nodes);

	bool pop_is_success = true;	// flag for Underflow checking

	uint32_t info;
	for (uint8_t operations = 0; operations < operations_factor; operations++) {
		for (uint16_t i = 0; i < max_nodes; i++){
			if (c_stack_push(stack, i) == false) {
				free(c_stack_memory);
				return 0;
			}
		}

		for (uint16_t i = 0; i < max_nodes; i++){
			info = c_stack_pop(stack, &pop_is_success);

			if (pop_is_success == false) {
				free(c_stack_memory);
				return 0;
			}
		}
	}

	free(c_stack_memory);
	return 1;
}
