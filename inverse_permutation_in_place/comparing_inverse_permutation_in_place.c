#include "main.h"
#include <string.h>


// Prototypes
void inverse_permutation_in_place(int32_t *permutation, uint32_t permutation_length);


void comparing_inverse_permutation_in_place() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	uint32_t permutation[] = {0, 6, 2, 1, 5, 4, 3};	// [0, 3, 2, 6, 5, 4, 1]
//	uint32_t permutation[] = {0, 4, 6, 2, 1, 3, 5};	// [0, 4, 3, 5, 1, 6, 2]
//	uint32_t permutation[] = {0, 1, 2, 5, 4, 3, 6};	// [0, 1, 2, 5, 4, 3, 6]
//	uint32_t permutation[] = {0, 6, 3, 2, 5, 1, 4};	// [0, 5, 3, 2, 6, 4, 1]

	// output: [0, 4, 10, 14, 13, 16, 9, 15, 8, 6, 12, 11, 7, 3, 2, 5, 1]
//	uint32_t permutation[] = {0, 16, 14, 13, 1, 15, 9, 12, 8, 6, 2, 11, 10, 4, 3, 7, 5};

	uint32_t permutation_length = (sizeof(permutation) / sizeof(permutation[0])); // n = 6;


	// GCC -O3
	// with uint32_t permutation[] = {0, 6, 2, 1, 5, 4, 3};
	// cycles_cold = [181-189], cycles_warm = 167;
	start = DWT->CYCCNT;
	inverse_permutation_in_place(permutation, permutation_length);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_inverse_permutation_in_place = (end - start) - overhead;

	start = DWT->CYCCNT;
	inverse_permutation_in_place(permutation, permutation_length);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_inverse_permutation_in_place = (end - start) - overhead;

	// ARM Assembly
	// uint32_t permutation[] = {0, 6, 2, 1, 5, 4, 3};
	// cycles_cold = [151], cycles_warm = 139;
	start = DWT->CYCCNT;
	asm_inverse_permutation_in_place(permutation, permutation_length);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_inverse_permutation_in_place = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_inverse_permutation_in_place(permutation, permutation_length);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_inverse_permutation_in_place = (end - start) - overhead;

	end = DWT->CYCCNT;
}
