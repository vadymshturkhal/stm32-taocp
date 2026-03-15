#include "main.h"


// Prototypes
void c_knuth_primes_mod_3_hack(uint32_t* primes_array, uint32_t primes_to_find);
void asm_knuth_primes_mod_3_hack(uint32_t* primes_array, uint32_t primes_to_find);

void comparing_primes() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint32_t PRIMES_TO_PRINT = 500;

	// GCC -O3
	// with 500 primes;
	// cycles_cold = [126198-126699], cycles_warm = [126163-126696], size = 92 bytes;
	volatile uint32_t* c_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	start = DWT->CYCCNT;
	c_knuth_primes_mod_3_hack(c_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_knuth_primes = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_knuth_primes_mod_3_hack(c_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_knuth_primes = (end - start) - overhead;


	// ARM Assembly
	// with 500 primes;
	// cycles_cold = [121038-121047], cycles_warm = [121009-121046], size = 84 bytes;
	volatile uint32_t* asm_primes_array = malloc(PRIMES_TO_PRINT * sizeof(uint32_t));
	start = DWT->CYCCNT;
	asm_knuth_primes_mod_3_hack(asm_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_knuth_primes = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_knuth_primes_mod_3_hack(asm_primes_array, PRIMES_TO_PRINT);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_knuth_primes = (end - start) - overhead;


	free(c_primes_array);
	free(asm_primes_array);
}
