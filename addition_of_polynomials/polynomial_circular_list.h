#ifndef POLYNOMIAL_CIRCULAR_LIST
#define POLYNOMIAL_CIRCULAR_LIST

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "storage_pool.h"

typedef struct PolynomialNode {
    struct PolynomialNode* link;
    int32_t COEFF;

//    int16_t sign;
//    int16_t A;
//    int16_t B;
//    int16_t C;
//    int64_t ABC;  // according to 4 * (sign type)

    int8_t sign;
    int8_t A;
    int8_t B;
    int8_t C;
    int32_t ABC;  // according to 4 * (sign type)
} PolynomialNode;

typedef struct {
	PolynomialNode* ptr;
	Storage_Pool* storage_pool;
} PolynomialCircularList;

typedef struct {
	int32_t COEFF;

//    int16_t sign;
//    int16_t A;
//    int16_t B;
//    int16_t C;

    int8_t sign;
    int8_t A;
    int8_t B;
    int8_t C;
} NodeInfo;

typedef struct {
	NodeInfo* P_terms;
	NodeInfo* Q_terms;
	uint32_t polynomial_P_size;
	uint32_t polynomial_Q_size;
} PolynomialsData;

typedef struct {
	PolynomialCircularList* polynomial_P;
	PolynomialCircularList* polynomial_Q;
	void* starting_address;
} Polynomials;

// Prototypes
void polynomial_circular_list_init(PolynomialCircularList* circular_list, Storage_Pool* storage_pool);
bool polynomial_circular_list_insert_left(PolynomialCircularList* circular_list, NodeInfo info);
bool polynomial_circular_list_insert_right(PolynomialCircularList* circular_list, NodeInfo info);
//uint32_t storage_circular_list_pop(bool* pop_is_success, CircularListStorage* circular_list);
//void storage_circular_list_clear(CircularListStorage* circular_list);
//void storage_circular_list_union(CircularListStorage* circular_list_a, CircularListStorage* circular_list_b);

#endif
