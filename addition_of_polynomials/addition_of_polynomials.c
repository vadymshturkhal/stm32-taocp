#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "main.h"
#include "polynomial_circular_list.h"
#include "storage_pool.h"

// A, B, C must be in int8_t range

// Return 0 if OK
// Return 1 if storage_pool Overflow
uint8_t addition_of_polynomials(PolynomialNode* P, PolynomialNode* Q, Storage_Pool* storage_pool) {
	PolynomialNode* Q1;

A1:
	// A1 [Initialize]
	P = P->link;
	Q1 = Q;
	Q = Q->link;

	while (1) {
		// A2. [ABC(P):ABC(Q)]
		while (P->ABC < Q->ABC) {
			Q1 = Q;
			Q = Q->link;
			continue;
		}

		if (P->ABC > Q->ABC) {
			// A5. [Insert new term]
			// Q2 <= AVAIL
			// FIXME allow COEFF 0
			PolynomialNode* Q2 = storage_pool_pop(storage_pool);
			if (Q2 == NULL) return 1;  // Overflow

			Q2->COEFF = P->COEFF;
			Q2->A = P->A;
			Q2->B = P->B;
			Q2->C = P->C;
			Q2->ABC = P->ABC;

			Q2->link = Q;
			Q1->link = Q2;
			Q1 = Q2;
			P = P->link;

			// Go to step 2
			continue;
		}

		// A3. [Add coefficients]
		if (P->ABC < 0) return 0;  // OK

		Q->COEFF += P->COEFF;
		if (Q->COEFF != 0) {
			goto A1;

			// or
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

		P = P->link;

		// Go to A2
		continue;
	}
}
