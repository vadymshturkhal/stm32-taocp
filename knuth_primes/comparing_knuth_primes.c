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

	// GCC -O3
	// with 500 primes;
	// cycles_cold = [147911], cycles_warm = 147909, size = 76 bytes;
	volatile uint32_t* c_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	start = DWT->CYCCNT;
	c_knuth_primes(c_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_knuth_primes = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_knuth_primes(c_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_knuth_primes = (end - start) - overhead;


	// ARM Assembly
	// with 500 primes;
	// cycles_cold = [141548-141561], cycles_warm = [141561-141563], size = 56 bytes;
	volatile uint32_t* asm_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	start = DWT->CYCCNT;
	asm_knuth_primes2(asm_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_knuth_primes = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_knuth_primes2(asm_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_knuth_primes = (end - start) - overhead;

	free(c_primes_array);
	free(asm_primes_array);
}
