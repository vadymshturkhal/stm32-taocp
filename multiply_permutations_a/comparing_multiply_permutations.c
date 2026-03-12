#include "main.h"
#include <string.h>


// Prototypes
void multiply_permutations_a_hack(char *permutation, uint32_t permutation_length, char *multiplication_result);
void asm_multiply_permutations_a1(char *permutation, uint32_t permutation_length, char *multiplication_result);

// Example of multiplication:
	// (acf)(bd)(abd)(ef) = (acefb);
	// (acf) means a->c->f->a;


void comparing_multiply_permutations() {
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

//	char *permutation = "(acf)(bd)";
//	char *permutation = "(acf)(bd)(abd)(ef)";
	char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
//	char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)(dgeac)";
	size_t permutation_length = strlen(permutation);

	// GCC Optimization -O2:
	// cycles with if = 3616;
	// cycles without if = 3115;
	// cycles without if and without singleton = 3309;
	// cycles without if and without singleton using Knuth style = 3157;

	// GCC -O3
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles cold = [2977-2983], cycles warm = 2917, size = 236 bytes;
	volatile char *multiplication_result_hack = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_hack_multiply_permutations = (end - start) - overhead;

	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_hack_multiply_permutations = (end - start) - overhead;


	// ARM Assembly
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles cold = [2613-2618], cycles warm = 2547, size = 88 bytes;
	volatile char *multiplication_result_asm1 = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	asm_multiply_permutations_a1(permutation, permutation_length, multiplication_result_asm1);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_multiply_permutations_a_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_multiply_permutations_a1(permutation, permutation_length, multiplication_result_asm1);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_multiply_permutations_a_cold = (end - start) - overhead;


	// free
	free(multiplication_result_hack);
	free(multiplication_result_asm1);
}
