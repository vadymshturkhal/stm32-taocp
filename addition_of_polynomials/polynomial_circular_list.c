#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "storage_pool.h"
#include "polynomial_circular_list.h"

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

void unpack_ABC_32(PolynomialNode* P, NodeInfo* info) {
	int32_t ABC = P->ABC;
	info->COEFF= P->COEFF;
    info->sign = (ABC >> 24) & 0xFF;
    info->A    = (ABC >> 16) & 0xFF;
    info->B    = (ABC >> 8)  & 0xFF;
    info->C    = ABC         & 0xFF;
}

//int32_t polynomial_result_unpack(PolynomialCircularList* polynomial_Q, NodeInfo* infos[]) {
//	if (polynomial_Q == NULL || polynomial_Q->ptr == NULL || info == NULL) return 1;
//
//	// find polynomial size
//	PolynomialNode* Q = polynomial_Q->ptr->link;
//	uint32_t size = 0;
//	while (Q->ABC > 0) {
//		size++;
//		Q = Q->link;
//	}
//
//	NodeInfo result_polynomial[size];
//
//	// take the result
//	Q = polynomial_Q->ptr->link;
//	uint32_t i = 0;
//	while (Q->ABC > 0) {
//		NodeInfo* info = result_polynomial[i++];
//
//		int32_t ABC = Q->ABC;
//		info->COEFF= Q->COEFF;
//	    info->sign = (ABC >> 24) & 0xFF;
//	    info->A    = (ABC >> 16) & 0xFF;
//	    info->B    = (ABC >> 8)  & 0xFF;
//	    info->C    = ABC         & 0xFF;
//
//		Q = Q->link;
//	}
//}
