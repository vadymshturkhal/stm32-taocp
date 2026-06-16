#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "polynomial_circular_list.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

// Return 0 if OK
// Return 1 if max_nodes <= 0
// Return 2 if not enough memory
uint8_t create_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials) {
	NodeInfo* P_terms = polynomials_data->P_terms;
	NodeInfo* Q_terms = polynomials_data->Q_terms;
	uint32_t polynomial_P_size = polynomials_data->polynomial_P_size;
	uint32_t polynomial_Q_size = polynomials_data->polynomial_Q_size;

	const uint32_t max_nodes = 2 + 4 * (polynomial_P_size + polynomial_Q_size);

	if (max_nodes <= 0) return 1;

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(PolynomialNode);
	uint32_t storage_and_circular_list_size = storage_pool_size + sizeof(PolynomialCircularList);

	void* storage_and_circular_list_memory = asm_balloc(storage_and_circular_list_size);
	if (storage_and_circular_list_memory == NULL) return 2;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_circular_list_memory, sizeof(PolynomialNode), max_nodes);

	uint8_t* circular_list_memory_start = (uint8_t*)storage_and_circular_list_memory + storage_pool_size;

	PolynomialCircularList* polynomial_P = (PolynomialCircularList*)(circular_list_memory_start);
	PolynomialCircularList* polynomial_Q = polynomial_P + 1;
	polynomial_circular_list_init(polynomial_P, storage_pool);
	polynomial_circular_list_init(polynomial_Q, storage_pool);

	for (uint32_t i = 0; i < polynomial_P_size; i++) {
		polynomial_circular_list_insert_right(polynomial_P, P_terms[i]);
	}

	for (uint32_t i = 0; i < polynomial_Q_size; i++) {
		polynomial_circular_list_insert_right(polynomial_Q, Q_terms[i]);
	}

	polynomials->polynomial_P = polynomial_P;
	polynomials->polynomial_Q = polynomial_Q;
	polynomials->starting_address = storage_and_circular_list_memory;

	return 0;
}
