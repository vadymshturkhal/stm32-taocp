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

//	C

//	// cycles = 3350, volatile removed
//	volatile char *multiplication_result = (char *)malloc(permutation_length * sizeof(char));
//	start = DWT->CYCCNT;
//	multiply_permutations_a(permutation, permutation_length, multiplication_result);
//	end = DWT->CYCCNT;
//	volatile uint32_t c_multiply_permutations_cycles = (end - start) - overhead;

	// GCC Optimization -O2:
	// cycles with if = 3616;
	// cycles without if = 3115;
	// cycles without if and without singleton = 3309;
	// cycles without if and without singleton using Knuth style = 3157;
	// GCC -O3
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles cold = [2978-2983], cycles warm = 2917
	volatile char *multiplication_result_hack = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t cycles_c_hack_multiply_permutations_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	multiply_permutations_a_hack(permutation, permutation_length, multiplication_result_hack);
	end = DWT->CYCCNT;
	volatile uint32_t cycles_c_hack_multiply_permutations_warm = (end - start) - overhead;


	// ARM Assembly
	// with char *permutation = "(acfg)(bcd)(aed)(fade)(bgfae)";
	// cycles cold = [2999-3004], cycles warm = [2916-2917]
//	volatile char *multiplication_result_asm = (char *)malloc(permutation_length * sizeof(char));
//	start = DWT->CYCCNT;
//	asm_multiply_permutations_a(permutation, permutation_length, multiplication_result_asm);
//	end = DWT->CYCCNT;
//	volatile uint32_t cycles_asm_multiply_permutations_a = (end - start) - overhead;

	// cycles cold = [2958 - 2963], cycles warm = [2874-2875]
	volatile char *multiplication_result_asm1 = (char *)malloc(permutation_length * sizeof(char));
	start = DWT->CYCCNT;
	asm_multiply_permutations_a1(permutation, permutation_length, multiplication_result_asm1);
	end = DWT->CYCCNT;
	volatile uint32_t cycles_asm_multiply_permutations_a_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_multiply_permutations_a1(permutation, permutation_length, multiplication_result_asm1);
	end = DWT->CYCCNT;
	volatile uint32_t cycles_asm_multiply_permutations_a_warm = (end - start) - overhead;

	// GCC is winning in cases like char *permutation = "(acf)(bd)";
	// ASM in other cases;

	// free
//	free(multiplication_result);
	free(multiplication_result_hack);
//	free(multiplication_result_asm);
	free(multiplication_result_asm1);
}
