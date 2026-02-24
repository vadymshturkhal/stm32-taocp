#include "main.h"
#include <string.h>


// Prototypes
void multiply_permutations_a(char *permutation, uint32_t permutation_length, char *multiplication_result);

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

//	volatile char *permutation = "(acf)(bd)(abd)(ef)";
	volatile char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	volatile size_t permutation_length = strlen(permutation);
	volatile char *multiplication_result = (char *)malloc(permutation_length * sizeof(char));

	// cycles = 4438
	start = DWT->CYCCNT;
	multiply_permutations_a(permutation, permutation_length, multiplication_result);
	end = DWT->CYCCNT;
	volatile uint32_t c_multiply_permutations_cycles = (end - start) - overhead;

	// free
	free(multiplication_result);
}
