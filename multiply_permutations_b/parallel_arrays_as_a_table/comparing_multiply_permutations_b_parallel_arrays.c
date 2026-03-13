#include "main.h"
#include <string.h>


// Example of multiplication:
	// (acf)(bd)(abd)(ef) = (acefb);
	// (acf) means a->c->f->a;


// Prototypes
multiply_permutations_b_parallel_arrays(char *permutation, uint32_t permutation_length, char *multiplication_result);
asm_multiply_permutations_b_parallel_arrays(char *permutation, uint32_t permutation_length, char *multiplication_result);


void comparing_multiply_permutations_b_parallel_arrays() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// permutation = "(acf)(bd)(abd)(ef)";
	// output = "(acefb)(d)"

    // permutation = "(acfg)(bcd)(aed)(fade)(bgfae)"
    // output = "(adg)(ceb)(f)"

    // permutation ="(acfg)(bcd)(aed)(fade)(bgfae)(dgeac)"
    // output = "(agc)(f)(bde)""

//	char *permutation = "(acf)(bd)";  // (acf)(bd)
//	char *permutation = "(acf)(bd)(abd)(ef)";
	char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
//	char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)(dgeac)";
	size_t permutation_length = strlen(permutation);


	// GCC -O3
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles_cold = [1984-1996], cycles_warm = 1913, size = 292 bytes;
	volatile char *c_multiplication_result = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_b_parallel_arrays(permutation, permutation_length, c_multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_multiply_permutations_b_parallel_arrays = (end - start) - overhead;

	start = DWT->CYCCNT;
	multiply_permutations_b_parallel_arrays(permutation, permutation_length, c_multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_multiply_permutations_b_parallel_arrays = (end - start) - overhead;

	// ARM Assembly
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles_cold = [1775-1784], cycles_warm = 1689, size = 342 bytes;
	volatile char *asm_multiplication_result = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	asm_multiply_permutations_b_parallel_arrays(permutation, permutation_length, asm_multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_multiply_permutations_b_parallel_arrays = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_multiply_permutations_b_parallel_arrays(permutation, permutation_length, asm_multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_multiply_permutations_b_parallel_arrays = (end - start) - overhead;


	// free
	free(c_multiplication_result);
	free(asm_multiplication_result);
}
