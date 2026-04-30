#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_circular_list.h"
#include "main.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);


uint8_t c_perform_circular_list_operations(uint32_t max_nodes) {
	// node info is uint32_t

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	if (max_nodes == 0) return 0;

	void* c_circular_list_memory = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList));
	if (c_circular_list_memory == NULL) return 0;

	start = DWT->CYCCNT;
	CircularList* circular_list = c_create_circular_list(c_circular_list_memory, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t c_create_circular_list_cycles_cold = (end - start) - overhead;

	uint32_t info;

	start = DWT->CYCCNT;
	// max_nodes times circular_list_insert_left
	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list_insert_left(circular_list, i) == false) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}
	end = DWT->CYCCNT;
	volatile uint32_t circular_list_insert_left_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	// max_nodes times circular_list_pop
	bool pop_is_success = true;		// flag for Underflow checking
	for (uint32_t i = max_nodes; i > 0; i--){
		info = circular_list_pop(circular_list, &pop_is_success);
		if (pop_is_success == false) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}

	end = DWT->CYCCNT;
	volatile uint32_t circular_list_pop_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	// max_nodes times circular_list_insert_right
	for (uint32_t i = max_nodes; i > 0; i--){
		if (circular_list_insert_right(circular_list, i) == false) {
			asm_balloc_free(c_circular_list_memory);
			return 0;
		}
	}

	end = DWT->CYCCNT;
	volatile uint32_t circular_list_insert_right_cycles_cold = (end - start) - overhead;

	circular_list_clear(circular_list);

	asm_balloc_free(c_circular_list_memory);
	return 1;
}
