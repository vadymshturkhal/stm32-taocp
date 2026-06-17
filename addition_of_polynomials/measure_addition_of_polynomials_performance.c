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
extern uint8_t asm_addition_of_polynomials_v1(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);
extern uint8_t asm_addition_of_polynomials_integrated_avail(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);


uint8_t create_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials);
uint8_t addition_of_polynomials(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);
uint8_t addition_of_polynomials_integrated_avail(PolynomialCircularList* polynomial_P, PolynomialCircularList* polynomial_Q);
//

uint8_t create_same_polynomials(
		void** starting_address,
		PolynomialsData* polynomials_data,
		Polynomials* polynomials,
		PolynomialCircularList** polynomial_P,
		PolynomialCircularList** polynomial_Q
) {
	asm_balloc_free(*starting_address);

	int32_t creation_status = create_polynomials(polynomials_data, polynomials);
	if (creation_status != 0) return 1;

	*polynomial_P = polynomials->polynomial_P;
	*polynomial_Q = polynomials->polynomial_Q;
	*starting_address = polynomials->starting_address;

	return 0;
}

uint8_t measure_addition_of_polynomials_performance(void) {
    volatile uint32_t start, end, overhead;
    uint8_t creation_status;
    uint8_t addition_status;


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
	creation_status = create_polynomials(&polynomials_data, &polynomials);
	if (creation_status != 0) return 1;

	PolynomialCircularList* polynomial_P = polynomials.polynomial_P;
	PolynomialCircularList* polynomial_Q = polynomials.polynomial_Q;
	void* starting_address = polynomials.starting_address;

	// Translation Unit Boundary Stats for TC4 with TC4_P_SIZE = 128 nodes and TC4_Q_SIZE = 128 nodes
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 4609-4726 | warm cycles = 4440-4444 | size = 140 bytes (addition_of_polynomials only)

	// Integrated AVAIL Stats for TC4 with TC4_P_SIZE = 128 nodes and TC4_Q_SIZE = 128 nodes
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 3551 | warm cycles = 3512 | size = 148 bytes (addition_of_polynomials only)
	start = DWT->CYCCNT;
//	addition_status = addition_of_polynomials(polynomial_P, polynomial_Q);
	addition_status = addition_of_polynomials_integrated_avail(polynomial_P, polynomial_Q);
	if (addition_status != 0) return addition_status;
	end = DWT->CYCCNT;
	volatile uint32_t addition_of_polynomials_cycles_cold = (end - start) - overhead;

	// Create same polynomial_P and polynomial_Q
	creation_status = create_same_polynomials(&starting_address, &polynomials_data, &polynomials, &polynomial_P, &polynomial_Q);
	if (creation_status != 0) return 1;

	// Warm run
	start = DWT->CYCCNT;
//	addition_status = addition_of_polynomials(polynomial_P, polynomial_Q);
	addition_status = addition_of_polynomials_integrated_avail(polynomial_P, polynomial_Q);
	if (addition_status != 0) return addition_status;
	end = DWT->CYCCNT;
	volatile uint32_t addition_of_polynomials_cycles_warm = (end - start) - overhead;

	// Create same polynomial_P and polynomial_Q
	creation_status = create_same_polynomials(&starting_address, &polynomials_data, &polynomials, &polynomial_P, &polynomial_Q);
	if (creation_status != 0) return 1;

	// Translation Unit Boundary Stats for TC4 with TC4_P_SIZE = 128 nodes and TC4_Q_SIZE = 128 nodes
	// ASM: cold cycles = 4390 | warm cycles = 4353 | size = 124 bytes (addition_of_polynomials only)
	// ASM: cold cycles = 4351 | warm cycles = 4313 | size = 124 bytes (addition_of_polynomials only) Placed P = P->link; between polynomial_Q->size--; in A4
	// ASM: cold cycles = 4305 | warm cycles = 4266 | size = 124 bytes (addition_of_polynomials only) Placed load polynomial size before P = P->link; in A5
	// ASM: cold cycles = 4259 | warm cycles = 4219 | size = 124 bytes (addition_of_polynomials only) Placed load polynomial size before Q2->link = Q; and advance polynomial size before P = P->link; in A5
	// ASM: cold cycles = 4219 | warm cycles = 4180 | size = 124 bytes (addition_of_polynomials only) Placed .balign 4 before A1

	// Integrated AVAIL Stats for TC4 with TC4_P_SIZE = 128 nodes and TC4_Q_SIZE = 128 nodes
	// ASM: cold cycles = 3437 | warm cycles = 3400 | size = 120 bytes
	start = DWT->CYCCNT;
	addition_status = asm_addition_of_polynomials_integrated_avail(polynomial_P, polynomial_Q);
	if (addition_status != 0) return addition_status;
	end = DWT->CYCCNT;
	volatile uint32_t asm_addition_of_polynomials_cycles_cold = (end - start) - overhead;

	// Create same polynomial_P and polynomial_Q
	creation_status = create_same_polynomials(&starting_address, &polynomials_data, &polynomials, &polynomial_P, &polynomial_Q);
	if (creation_status != 0) return 1;

	start = DWT->CYCCNT;
	addition_status = asm_addition_of_polynomials_integrated_avail(polynomial_P, polynomial_Q);
	if (addition_status != 0) return addition_status;
	end = DWT->CYCCNT;
	volatile uint32_t asm_addition_of_polynomials_cycles_warm = (end - start) - overhead;

//	// take the result
//	PolynomialNode* Q = polynomial_Q->ptr->link;
//	volatile NodeInfo result_polynomial[polynomial_Q->size-1];  // Without last node
//	Q = polynomial_Q->ptr->link;
//	uint32_t i = 0;
//	while (Q->ABC > 0) {
//		unpack_ABC_32(Q, &result_polynomial[i++]);
//
//		// This is equivalent to &result_polynomial[i++]
////		unpack_ABC_32(Q, result_polynomial + i);
////		i++;
//
//		Q = Q->link;
//	}
//	//

	asm_balloc_free(starting_address);
}
