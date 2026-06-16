#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "storage_pool.h"
#include "polynomial_circular_list.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
uint8_t create_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials);
uint8_t addition_of_polynomials(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);

uint8_t measure_addition_of_polynomials_performance(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

//	const uint32_t polynomial_P_size = 4;
//	const uint32_t polynomial_Q_size = 4;
//	NodeInfo P_terms[] = {
//		{ .COEFF = 1, .sign = 1, .A = 1, .B = 0, .C = 0 },  // x
//		{ .COEFF = 1, .sign = 1, .A = 0, .B = 1, .C = 0 },  // y
//		{ .COEFF = 1, .sign = 1, .A = 1, .B = 0, .C = 1 },  // xz
//		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
//	};
//
//	NodeInfo Q_terms[] = {
//		{ .COEFF = 1, .sign = 1, .A = 2, .B = 0, .C = 0 },  // x^2
//		{ .COEFF = -2, .sign = 1, .A = 0, .B = 1, .C = 0 },  // -2y
//		{ .COEFF = -1, .sign = 1, .A = 0, .B = 0, .C = 1 },  // -z
//		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
//	};

//	const uint32_t polynomial_P_size = 3;
//	const uint32_t polynomial_Q_size = 3;
//	NodeInfo P_terms[] = {
//		{ .COEFF = 1, .sign = 1, .A = 3, .B = 0, .C = 0 },  // x^3
//		{ .COEFF = 1, .sign = 1, .A = 0, .B = 0, .C = 2 },  // z^2
//		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
//	};
//
//	NodeInfo Q_terms[] = {
//		{ .COEFF = 1, .sign = 1, .A = 0, .B = 3, .C = 0 },  // y^3
//		{ .COEFF = 1, .sign = 1, .A = 0, .B = 0, .C = 2 },  // z^2
//		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
//	};

	const uint32_t polynomial_P_size = 4;
	const uint32_t polynomial_Q_size = 3;
	NodeInfo P_terms[] = {
		{ .COEFF = 1, .sign = 1, .A = 3, .B = 0, .C = 0 },  // x^3
		{ .COEFF = 1, .sign = 1, .A = 2, .B = 0, .C = 0 },  // x^2
		{ .COEFF = 1, .sign = 1, .A = 0, .B = 0, .C = 2 },  // z^2
		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
	};

	NodeInfo Q_terms[] = {
		{ .COEFF = 1, .sign = 1, .A = 0, .B = 3, .C = 0 },  // y^3
		{ .COEFF = 1, .sign = 1, .A = 0, .B = 0, .C = 2 },  // z^2
		{ .COEFF = 0, .sign = -1, .A = 0, .B = 0, .C = 1 },  // last node
	};

	PolynomialsData polynomials_data = {
		.P_terms = P_terms,
		.Q_terms = Q_terms,
		.polynomial_P_size = polynomial_P_size,
		.polynomial_Q_size = polynomial_Q_size,
	};

	Polynomials polynomials;
	uint8_t creation_status = create_polynomials(&polynomials_data, &polynomials);
	if (creation_status != 0) return 1;

	PolynomialCircularList* polynomial_P = polynomials.polynomial_P;
	PolynomialCircularList* polynomial_Q = polynomials.polynomial_Q;
	void* starting_address = polynomials.starting_address;

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = ? | warm cycles = ? | size = ? bytes
	start = DWT->CYCCNT;
	addition_of_polynomials(polynomial_P, polynomial_Q);
	end = DWT->CYCCNT;
	volatile uint32_t addition_of_polynomials_cycles_cold = (end - start) - overhead;

//	start = DWT->CYCCNT;
//	addition_of_polynomials(polynomial_P->ptr, polynomial_Q->ptr, polynomial_P->storage_pool);
//	end = DWT->CYCCNT;
//	volatile uint32_t addition_of_polynomials_cycles_warm = (end - start) - overhead;

	// take the result
	PolynomialNode* Q = polynomial_Q->ptr->link;
	NodeInfo result_polynomial[polynomial_Q->size-1];  // Without last node
	Q = polynomial_Q->ptr->link;
	uint32_t i = 0;
	while (Q->ABC > 0) {
		unpack_ABC_32(Q, &result_polynomial[i++]);
		Q = Q->link;
	}



	asm_balloc_free(starting_address);
}
