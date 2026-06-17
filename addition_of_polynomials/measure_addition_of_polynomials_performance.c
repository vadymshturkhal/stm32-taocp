#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "storage_pool.h"
#include "polynomial_circular_list.h"
#include "polynomial_test_cases.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
extern uint8_t asm_addition_of_polynomials(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);

uint8_t create_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials);
uint8_t addition_of_polynomials(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);
//

uint8_t measure_addition_of_polynomials_performance(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// test case
	const uint32_t polynomial_P_size = TC4_P_SIZE;
	const uint32_t polynomial_Q_size = TC4_Q_SIZE;
	PolynomialsData polynomials_data = {
		.P_terms = TC4_P_terms,
		.Q_terms = TC4_Q_terms,
		.polynomial_P_size = polynomial_P_size,
		.polynomial_Q_size = polynomial_Q_size,
	};

	Polynomials polynomials;
	uint8_t creation_status = create_polynomials(&polynomials_data, &polynomials);
	if (creation_status != 0) return 1;

	PolynomialCircularList* polynomial_P = polynomials.polynomial_P;
	PolynomialCircularList* polynomial_Q = polynomials.polynomial_Q;
	void* starting_address = polynomials.starting_address;

	// Stats for TC4 with TC4_P_SIZE = 128 nodes and TC4_Q_SIZE = 128 nodes
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 4726 | warm cycles = 4444 | size = 140 bytes (addition_of_polynomials only)
	start = DWT->CYCCNT;
	addition_of_polynomials(polynomial_P, polynomial_Q);
	end = DWT->CYCCNT;
	volatile uint32_t addition_of_polynomials_cycles_cold = (end - start) - overhead;

	// Create same polynomial_P and polynomial_Q
	asm_balloc_free(starting_address);
	create_polynomials(&polynomials_data, &polynomials);
	if (creation_status != 0) return 1;
	polynomial_P = polynomials.polynomial_P;
	polynomial_Q = polynomials.polynomial_Q;
	starting_address = polynomials.starting_address;

	// Warm run
	start = DWT->CYCCNT;
	addition_of_polynomials(polynomial_P, polynomial_Q);
	end = DWT->CYCCNT;
	volatile uint32_t addition_of_polynomials_cycles_warm = (end - start) - overhead;

	// take the result
	PolynomialNode* Q = polynomial_Q->ptr->link;
	volatile NodeInfo result_polynomial[polynomial_Q->size-1];  // Without last node
	Q = polynomial_Q->ptr->link;
	uint32_t i = 0;
	while (Q->ABC > 0) {
		unpack_ABC_32(Q, &result_polynomial[i++]);

		// This is equivalent to &result_polynomial[i++]
//		unpack_ABC_32(Q, result_polynomial + i);
//		i++;

		Q = Q->link;
	}

	start = DWT->CYCCNT;
	asm_addition_of_polynomials(polynomial_P, polynomial_Q);
	end = DWT->CYCCNT;
	volatile uint32_t asm_addition_of_polynomials_cycles_cold = (end - start) - overhead;



	asm_balloc_free(starting_address);
}
