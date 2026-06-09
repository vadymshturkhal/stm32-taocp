#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "storage_pool.h"
#include "xor_circular_list.h"


// Using Storage Pool

void xor_circular_list_init(XORCircularList* circular_list, Storage_Pool* storage_pool) {
    XORCircularNodeHead* head1 = (XORCircularNodeHead*)circular_list;
    XORCircularNodeHead* head2 = head1 + 1;

    head1->link = 0;
    head2->link = 0;

    circular_list->storage_pool = storage_pool;
}

bool xor_circular_list_insert_left(XORCircularList* circular_list, uint32_t info) {
    if (circular_list == NULL) return false;

    XORCircularNode* P = storage_pool_pop(circular_list->storage_pool);
    if (P == NULL) return false;

    XORCircularNodeHead* head1 = (XORCircularNodeHead*)circular_list;
    XORCircularNodeHead* head2 = head1 + 1;

    uintptr_t head1_address = (uintptr_t)head1;
    uintptr_t head2_address = (uintptr_t)head2;
    uintptr_t P_address  = (uintptr_t)P;

    uintptr_t old_first_addr = head2->link ^ head1_address;

    if (circular_list->size == 0) {
        P->link = head1_address ^ head2_address;
        head2->link = P_address ^ head1_address;
    } else {
        XORCircularNode* old_first = (XORCircularNode*)old_first_addr;
        P->link = old_first_addr ^ head2_address;
        old_first->link = P_address ^ (old_first->link ^ head2_address);
    }

    head2->link = P_address ^ head1_address;

    P->info = info;

    circular_list->size++;
    return true;
}

uint32_t xor_circular_list_pop_left(bool* pop_is_success, XORCircularList* circular_list) {
    if (circular_list == NULL) return 0;

    if (circular_list->size == 0) {
		*pop_is_success = false;	// Underflow
		return 0;
    }

	*pop_is_success = true;

    XORCircularNodeHead* head1 = (XORCircularNodeHead*)circular_list;
    XORCircularNodeHead* head2 = head1 + 1;

    uintptr_t head1_address = (uintptr_t)head1;
    uintptr_t head2_address = (uintptr_t)head2;

    uintptr_t front_node_address = head2->link ^ head1_address;
    XORCircularNode* front_node = (XORCircularNode*)front_node_address;
    uintptr_t front_neighbour_address = front_node->link ^ head2_address;

    if (front_neighbour_address == head1_address) {
        head1->link = 0;
        head2->link = 0;
    } else {
        XORCircularNode* front_neighbour = (XORCircularNode*)front_neighbour_address;
        head2->link = head1_address ^ front_neighbour_address;
        front_neighbour->link = (front_neighbour->link ^ front_node_address) ^ head2_address;
    }

    uint32_t info = front_node->info;
    storage_pool_push(circular_list->storage_pool, front_node);

    circular_list->size -= 1;
    return info;
}

// For O(1) Clear and Union operation there must be XOR Storage Pool
//void xor_circular_list_clear(XORCircularList* circular_list) {
//
//}

//void storage_circular_list_union(XORCircularList* circular_list_a, XORCircularList* circular_list_b) {
//
//}

