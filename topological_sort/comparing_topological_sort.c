#include <stdint.h>
#include "main.h"
#include "pairs.h"
#include "test_data.h"

// Prototypes
uint8_t c_algorithm_t(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);


void comparing_topological_sort() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

//	uint32_t output[n]; // would crash the program
	uint32_t* output = asm_balloc(n * sizeof(uint32_t));

	// all stats for the next case:
	// const uint32_t n = 9;
	// const uint32_t input_pairs_len = 10;

	// GCC -O3
	// cycles_cold = [1424-1432], cycles_warm = [1355-1356], size = 384 bytes
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




	// Free heap memory
	asm_balloc_free(output);
}
