#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "storage_pool.h"

#include "polynomial_circular_list.h"
#include "polynomial_test_cases.h"


// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint32_t create_three_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials);
uint32_t multiplication_of_polynomials(PolynomialCircularList* polynomial_Q, PolynomialCircularList* polynomial_M, PolynomialCircularList* polynomial_P);
uint32_t multiplication_of_polynomials_avail_integrated(PolynomialCircularList* polynomial_Q, PolynomialCircularList* polynomial_M, PolynomialCircularList* polynomial_P);


uint32_t measure_multiplication_of_polynomials_performance() {
    volatile uint32_t start, end, overhead;
    uint8_t creation_status;
    uint32_t multiplication_status;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// test case
	// Q is empty and M is proper Q
	const uint32_t polynomial_P_size = TC5_P_SIZE;
	const uint32_t polynomial_Q_size = EMPTY_SIZE;
	const uint32_t polynomial_M_size = TC5_Q_SIZE;
	PolynomialsData polynomials_data = {
		.P_terms = TC5_P_terms,
		.Q_terms = EMPTY_terms,
		.M_terms = TC5_Q_terms,
		.polynomial_P_size = polynomial_P_size,
		.polynomial_Q_size = polynomial_Q_size,
		.polynomial_M_size = polynomial_M_size,
	};

	Polynomials polynomials;
	creation_status = create_three_polynomials(&polynomials_data, &polynomials);
	if (creation_status != 0) return 1;

	PolynomialCircularList* polynomial_P = polynomials.polynomial_P;
	PolynomialCircularList* polynomial_Q = polynomials.polynomial_Q;
	PolynomialCircularList* polynomial_M = polynomials.polynomial_M;
	void* starting_address = polynomials.starting_address;

	// Translation Unit Boundary Stats (Algorithm M -> Multiplication Subroutine -> Storage Pool) TC1
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 788 | warm cycles = 718 | size = 212 bytes (multiplication_of_polynomials only)

	// Translation Unit Boundary AVAIL Integrated Stats (Algorithm M -> Multiplication Subroutine) TC1
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 677 | warm cycles = 612 | size = 204 bytes (multiplication_of_polynomials only)

	start = DWT->CYCCNT;
//	multiplication_status = multiplication_of_polynomials(polynomial_Q, polynomial_M, polynomial_P);
	multiplication_status = multiplication_of_polynomials_avail_integrated(polynomial_Q, polynomial_M, polynomial_P);
	if (multiplication_status != 0) return multiplication_status;
	end = DWT->CYCCNT;
	volatile uint32_t multiplication_of_polynomials_cycles_cold = (end - start) - overhead;

	polynomial_circular_list_clear(polynomial_Q);
	polynomial_circular_list_insert_right(polynomial_Q, EMPTY_term);

	// Warm run
	start = DWT->CYCCNT;
//	multiplication_status = multiplication_of_polynomials(polynomial_Q, polynomial_M, polynomial_P);
	multiplication_status = multiplication_of_polynomials_avail_integrated(polynomial_Q, polynomial_M, polynomial_P);
	if (multiplication_status != 0) return multiplication_status;
	end = DWT->CYCCNT;
	volatile uint32_t multiplication_of_polynomials_cycles_warm = (end - start) - overhead;


	// Showing result
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
	//


	asm_balloc_free(starting_address);
	return 0;
}
