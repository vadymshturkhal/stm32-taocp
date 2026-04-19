#include <stdint.h>
#include "main.h"
#include "pairs.h"
#include "test_data.h"

// Prototypes
uint8_t c_algorithm_t(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
extern uint8_t asm_algorithm_t(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);

void comparing_topological_sort() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// uint32_t output[n]; // would crash the program
	uint32_t* output = asm_balloc(n * sizeof(uint32_t));
	uint32_t* asm_output = asm_balloc(n * sizeof(uint32_t));

	// GCC -O3
	// cold cycles = 10363 | warm cycles = 10262 | size = 384 bytes
	start = DWT->CYCCNT;
	uint8_t topological_status = c_algorithm_t(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t c_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	topological_status = c_algorithm_t(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t c_topological_sort_cycles_warm = (end - start) - overhead;

	// ARM Assembly
	// cold cycles = 7351-7373 | warm cycles = 7307-7310 | size = 308 bytes
	start = DWT->CYCCNT;
	uint8_t asm_topological_status = asm_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_topological_status = asm_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_topological_sort_cycles_warm = (end - start) - overhead;

	// Free heap memory
	asm_balloc_free(asm_output);
	asm_balloc_free(output);
}
