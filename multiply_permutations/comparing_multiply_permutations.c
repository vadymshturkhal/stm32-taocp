#include "main.h"
#include <string.h>


// Prototypes
void multiply_permutations_a(char *permutation, uint32_t permutation_length, char *multiplication_result);
void multiply_permutations_a_hack(char *permutation, uint32_t permutation_length, char *multiplication_result);
void multiply_permutations_a_hack_asm(char *permutation, uint32_t permutation_length, char *multiplication_result);

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

	// cycles = 3350, volatile removed
	volatile char *multiplication_result = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a(permutation, permutation_length, multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t c_multiply_permutations_cycles = (end - start) - overhead;

	// cycles with if = 3616
	// cycles without if = 3115
	// cycles without if and without singleton = 3309
	// cycles without if and without singleton using Knuth style = 3157
	volatile char *multiplication_result_hack = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t c_hack_multiply_permutations_cycles = (end - start) - overhead;

	// cycles = 3167, with asm_hack1 = 3083 , with .balign 4 = 3083
	volatile char *multiplication_result_hack_asm = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a_hack_asm(permutation, permutation_length, multiplication_result_hack_asm);
	end = DWT->CYCCNT;
	volatile uint32_t asm_hack_multiply_permutations_cycles = (end - start) - overhead;




	// free
	free(multiplication_result);
	free(multiplication_result_hack);
	free(multiplication_result_hack_asm);
}

