#include "../polynomials/polynomial_circular_list.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "storage_pool.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

void polynomial_circular_list_init(PolynomialCircularList* circular_list, Storage_Pool* storage_pool) {
	circular_list->ptr = NULL;
	circular_list->storage_pool = storage_pool;
	circular_list->size = 0;
}

bool polynomial_circular_list_insert_left(PolynomialCircularList* circular_list, NodeInfo info) {
	// return false if Overflow, else true
	if (circular_list == NULL) return false;

	// 1 (P <= Avail)
	PolynomialNode* P = storage_pool_pop(circular_list->storage_pool);
	if (P == NULL) return false;	// Overflow

	// 2
	P->COEFF = info.COEFF;

	// for int8_t sign, A, B and C
	P->ABC = ((int32_t)info.sign << 24) |
	           ((int32_t)info.A    << 16) |
	           ((int32_t)info.B    << 8) |
	           ((int32_t)info.C);

	// for int16_t sign, A, B and C
//	 P->ABC = ((int64_t)info.sign << 48) |
//	           ((int64_t)info.A    << 32) |
//	           ((int64_t)info.B    << 16) |
//	           ((int64_t)info.C);

	// 3
	if (circular_list->ptr == NULL) {
		// List is currently empty, P points to itself
		P->link = P;
		circular_list->ptr = P;
	} else {
		// Insert P at the front
		P->link = circular_list->ptr->link;
		circular_list->ptr->link = P;
	}

	circular_list->size++;

	return true;
}

bool polynomial_circular_list_insert_right(PolynomialCircularList* circular_list, NodeInfo info) {
	if (!polynomial_circular_list_insert_left(circular_list, info)) return false;

	circular_list->ptr = circular_list->ptr->link;
	return true;
}

void polynomial_circular_list_clear(PolynomialCircularList* circular_list) {
	if (circular_list == NULL) return;

	if (circular_list->ptr != NULL) {
		PolynomialNode* head = circular_list->ptr->link;
		PolynomialNode* tail = circular_list->ptr;

		storage_pool_add_slice(circular_list->storage_pool, head, tail);
		circular_list->ptr = NULL;
		circular_list->size = 0;
	}
}

void unpack_ABC_32(PolynomialNode* P, NodeInfo* info) {
	int32_t ABC = P->ABC;
	info->COEFF= P->COEFF;
    info->sign = (ABC >> 24) & 0xFF;
    info->A    = (ABC >> 16) & 0xFF;
    info->B    = (ABC >> 8)  & 0xFF;
    info->C    = ABC         & 0xFF;
}

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
	uint32_t polynomials_size = 2 * sizeof(PolynomialCircularList);
	uint32_t storage_and_circular_list_size = storage_pool_size + polynomials_size;

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

uint32_t create_three_polynomials(PolynomialsData* polynomials_data, Polynomials* polynomials) {
	NodeInfo* P_terms = polynomials_data->P_terms;
	NodeInfo* Q_terms = polynomials_data->Q_terms;
	NodeInfo* M_terms = polynomials_data->M_terms;
	uint32_t polynomial_P_size = polynomials_data->polynomial_P_size;
	uint32_t polynomial_Q_size = polynomials_data->polynomial_Q_size;
	uint32_t polynomial_M_size = polynomials_data->polynomial_M_size;

	const uint32_t polynomials_nodes = polynomial_P_size + polynomial_Q_size + polynomial_M_size;
	const uint32_t max_Q_nodes = polynomial_Q_size + polynomial_P_size * polynomial_M_size - 2;  // -2 sentinels
	const uint32_t max_nodes = polynomials_nodes + max_Q_nodes;

	if (max_nodes <= 0) return 1;

	uint32_t storage_pool_size = sizeof(Storage_Pool) + max_nodes * sizeof(PolynomialNode);
	uint32_t polynomials_size = 3 * sizeof(PolynomialCircularList);
	uint32_t storage_and_circular_list_size = storage_pool_size + polynomials_size;

	void* storage_and_circular_list_memory = asm_balloc(storage_and_circular_list_size);
	if (storage_and_circular_list_memory == NULL) return 2;

	Storage_Pool* storage_pool = create_storage_pool(storage_and_circular_list_memory, sizeof(PolynomialNode), max_nodes);

	uint8_t* circular_list_memory_start = (uint8_t*)storage_and_circular_list_memory + storage_pool_size;

	PolynomialCircularList* polynomial_P = (PolynomialCircularList*)(circular_list_memory_start);
	PolynomialCircularList* polynomial_Q = polynomial_P + 1;
	PolynomialCircularList* polynomial_M = polynomial_Q + 1;
	polynomial_circular_list_init(polynomial_P, storage_pool);
	polynomial_circular_list_init(polynomial_Q, storage_pool);
	polynomial_circular_list_init(polynomial_M, storage_pool);

	for (uint32_t i = 0; i < polynomial_P_size; i++) {
		polynomial_circular_list_insert_right(polynomial_P, P_terms[i]);
	}

	for (uint32_t i = 0; i < polynomial_Q_size; i++) {
		polynomial_circular_list_insert_right(polynomial_Q, Q_terms[i]);
	}

	for (uint32_t i = 0; i < polynomial_M_size; i++) {
		polynomial_circular_list_insert_right(polynomial_M, M_terms[i]);
	}

	polynomials->polynomial_P = polynomial_P;
	polynomials->polynomial_Q = polynomial_Q;
	polynomials->polynomial_M = polynomial_M;
	polynomials->starting_address = storage_and_circular_list_memory;

	return 0;
}
