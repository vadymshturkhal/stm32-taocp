#include <stdint.h>
#include "main.h"
#include "pairs.h"
#include "test_data.h"

// Prototypes
uint8_t c_algorithm_t(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
uint8_t leetcode_regular_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);
uint8_t leetcode_hero_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output);


void comparing_c_topological_sort() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// uint32_t output[n]; // would crash the program
	uint32_t* output = asm_balloc(n * sizeof(uint32_t));

	// all stats for the next case:
	//const uint32_t n = 50;
	//const uint32_t input_pairs_len = 139;

	// Algorithm T
	// cycles_cold = 10478, cycles_warm = 10400, size = 384 bytes
	start = DWT->CYCCNT;
	uint8_t topological_status = c_algorithm_t(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t algorithm_t_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	topological_status = c_algorithm_t(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t algorithm_t_cycles_warm = (end - start) - overhead;

	// LeetCode Regular Topological Sort
	// cycles_cold = 47308, cycles_warm = 35891, size = 772 bytes
	start = DWT->CYCCNT;
	topological_status = leetcode_regular_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t leetcode_regular_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	topological_status = leetcode_regular_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t leetcode_regular_cycles_warm = (end - start) - overhead;

	// LeetCode Hero Topological Sort
	// cycles_cold = 22431, cycles_warm = 22385, size = 924 bytes
	start = DWT->CYCCNT;
	topological_status = leetcode_hero_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t leetcode_hero_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	topological_status = leetcode_hero_topological_sort(n, input_pairs, input_pairs_len, output);
	if (topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t leetcode_hero_cycles_warm = (end - start) - overhead;



	// Free heap memory
	asm_balloc_free(output);
}
