#include "main.h"


// Prototypes
uint32_t josephus_generalized(uint32_t mod, uint32_t participants);
uint32_t asm_josephus_generalized(uint32_t mod, uint32_t participants);

void comparing_josephus_n() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;


	uint32_t mod = 33;
	uint32_t participants = 10111;	// for mod 3 and participants 10 answer == 4
	volatile uint32_t c_survivor, asm_survivor;


	// GCC -O3
	// with mod 3 and participants 10;
	// cycles_cold = [354-356], cycles_warm = 298, size = 92 bytes;

	// GCC -O3
	// with mod 33 and participants 10111;
	// cycles_cold = 14006, cycles_warm = 13950, size = 92 bytes;
	start = DWT->CYCCNT;
	c_survivor = josephus_generalized(mod, participants);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_cold_josephus_n = (end - start) - overhead;

	start = DWT->CYCCNT;
	c_survivor = josephus_generalized(mod, participants);
	end = DWT->CYCCNT;
	volatile uint32_t c_cycles_warm_josephus_n = (end - start) - overhead;


	// ASM
	// with mod 3 and participants 10;
	// cycles_cold = 183, cycles_warm = 153, size = 114 bytes;

	// ASM
	// with mod 33 and participants 10111;
	// cycles_cold = 4846, cycles_warm = 4816, size = 114 bytes;
	start = DWT->CYCCNT;
	asm_survivor = asm_josephus_generalized(mod, participants);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold_josephus_n = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_survivor = asm_josephus_generalized(mod, participants);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm_josephus_n = (end - start) - overhead;




	end = DWT->CYCCNT;
}
