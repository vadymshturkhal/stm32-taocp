#include "main.h"
#include <string.h>


// Prototypes
void multiply_permutations_a(char *permutation, uint32_t permutation_length, char *multiplication_result);
void multiply_permutations_a_hack(char *permutation, uint32_t permutation_length, char *multiplication_result);

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

//	char *permutation = "(acf)(bd)(abd)(ef)";
	char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	size_t permutation_length = strlen(permutation);

	// cycles = 3350, volatile removed
	char *multiplication_result = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a(permutation, permutation_length, multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t c_multiply_permutations_cycles = (end - start) - overhead;

	// cycles = 3616 with if, cycles_without_if = 3115
	char *multiplication_result_hack = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t c_hack_multiply_permutations_cycles = (end - start) - overhead;

	// free
	free(multiplication_result);
	free(multiplication_result_hack);
}

