#include "main.h"


// Prototypes
uint32_t* c_knuth_primes(uint32_t* primes_array, uint32_t primes_to_find);
uint32_t* asm_knuth_primes(uint32_t* primes_array, uint32_t primes_to_find);

void comparing_primes() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	volatile uint32_t PRIMES_TO_PRINT = 500;

	// cycles = 167611 for 500 primes v.2
	start = DWT->CYCCNT;
	volatile uint32_t* c_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	c_knuth_primes(c_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t c_primes_cycles = (end - start) - overhead;

	// cycles = 158300 for 500 prime
	start = DWT->CYCCNT;
	volatile uint32_t* asm_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	asm_knuth_primes(asm_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t asm_primes_cycles = (end - start) - overhead;

	free(c_primes_array);
	free(asm_primes_array);
}
