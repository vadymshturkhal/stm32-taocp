#include <stdint.h>
#include "main.h"
#include "pairs.h"

// Prototypes
uint8_t c_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

void comparing_topological_sort() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// Run tests
	// Output: [9, 1, 2, 3, 7, 5, 4, 8, 6]
	const n = 9;
	const input_pairs_len = 10;
	Pair input_pairs[] = {
	        {9, 2},
	        {3, 7},
	        {7, 5},
	        {5, 8},
	        {8, 6},
	        {4, 6},
	        {1, 3},
	        {7, 4},
	        {9, 5},
	        {2, 8},
	};

	uint32_t* output = asm_balloc(n * sizeof(uint32_t));

	// GCC -O3
	// cycles_cold = 1408, cycles_warm = 1322, size = 376 bytes
	start = DWT->CYCCNT;
	uint8_t topological_status = c_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t c_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	topological_status = c_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t c_topological_sort_cycles_warm = (end - start) - overhead;




	// Free heap memory
	asm_balloc_free(output);
}
