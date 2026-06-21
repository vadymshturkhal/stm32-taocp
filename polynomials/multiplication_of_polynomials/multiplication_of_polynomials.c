#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "storage_pool.h"
#include "polynomial_circular_list.h"

#define SENTINEL_NODE_ABC -16777215;

// A, B, C must be in int8_t range
uint32_t multiply_cycle(PolynomialCircularList* polynomial_Q, PolynomialNode* M, PolynomialCircularList* polynomial_P) {
    // Based of Algorithm A (Addition of polynomials)
    // Return status

	PolynomialNode* Q = polynomial_Q->ptr;
	PolynomialNode* P = polynomial_P->ptr;
	PolynomialNode* Q1;
	Storage_Pool* storage_pool = polynomial_Q->storage_pool;

A1:
	// A1. [Initialize]
	P = P->link;
	Q1 = Q;
	Q = Q->link;

	while (1) {
		// A2. [ABC(P):ABC(Q)]
		int32_t ABC;
		if (P->ABC < 0) {
			// ABC = -1;
			ABC = SENTINEL_NODE_ABC;
		} else {
			ABC = P->ABC + M->ABC;
		}

		int32_t COEFF = P->COEFF * M->COEFF;

		while (ABC < Q->ABC) {
			Q1 = Q;
			Q = Q->link;
		}

		if (ABC > Q->ABC) {
			// A5. [Insert new term]
			// Q2 <= AVAIL
			PolynomialNode* Q2 = storage_pool_pop(storage_pool);
			if (Q2 == NULL) return 1;  // Overflow

			Q2->COEFF = COEFF;
			Q2->ABC = ABC;
			Q2->link = Q;
			Q1->link = Q2;
			Q1 = Q2;
			P = P->link;

			polynomial_Q->size++;

			// Go to step 2
			continue;
		}

		// A3. [Add coefficients]
		if (ABC < 0) return 0;  // OK

		Q->COEFF += COEFF;
		if (Q->COEFF != 0) {
			// goto A1 is my own trick, pointed out by Knuth
			goto A1;

			// instead of goto one can use next steps from Algorithm A
			// P = P->link;
			// Q1 = Q;
			// Q = Q->link;

			// Go to A2
			//continue;
		}

		// A4. [Delete zero term]
		PolynomialNode* Q2 = Q;
		Q = Q->link;
		Q1->link = Q;

		// AVAIL <= Q2
		storage_pool_push(storage_pool, Q2);
		polynomial_Q->size--;

		P = P->link;

		// Go to A2
		continue;
	}
}

// Return 0 if OK
// Return 1 if storage_pool Overflow
uint32_t multiplication_of_polynomials(PolynomialCircularList* polynomial_Q, PolynomialCircularList* polynomial_M, PolynomialCircularList* polynomial_P) {
    // Result: Polynomial Q + Polynomial M x Polynomial P
    // Return: status

	PolynomialNode* M = polynomial_M->ptr;

	// M1. [Next multiplier]
	M = M->link;

	while (M->ABC >= 0) {
		// M2. [Multiply cycle]
		uint32_t status = multiply_cycle(polynomial_Q, M, polynomial_P);
		if (status != 0) return status;

		// M1. [Next multiplier]
		M = M->link;
	} ;

//	do {
//		// M1. [Next multiplier]
//		M = M->link;
//
//		// M2. [Multiply cycle]
//		uint32_t status = multiply_cycle(polynomial_Q, M, polynomial_P);
//		if (status != 0) return status;
//	} while (M->ABC >= 0);

	return 0;
}
