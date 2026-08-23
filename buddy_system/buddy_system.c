#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "buddy_system.h"

uint32_t buddy_system_init(BuddySystem* system, void* memory, uint32_t m) {
	// memory is a starting memory of MCU
	if (system == NULL || memory == NULL || m == 0) return 1;

	uint32_t arena_size = 1u << m;
	uint32_t lists_size = (m + 1) * sizeof(BuddyList);

	BuddyNode* block = (BuddyNode*)memory;
	BuddyList* list = (BuddyList*)((uint8_t*)memory + arena_size);
	BuddyNode* head = (BuddyNode*)((uint8_t*)list + lists_size);

	system->list = list;
	system->base = (uint8_t*)memory;
	system->m = m;

	// AVAIL[0], AVAIL[1],...,AVAIL[m]
	for (uint32_t k = 0; k <= m; k++) {
		uint32_t status = buddy_list_init(&list[k], &head[k], k);
		if (status != 0) return status;
	}

	// AVAILF[m] = AVAILB[m] = 0
	// LINKF(0) = LINKB(0) = LOC(AVAIL[m])
	return buddy_list_insert(block, &list[m]);
}

// Emulated system->base memory starts at 0 
BuddyNode* buddy_address(BuddySystem* system, BuddyNode* node, uint32_t k) {
	uint32_t offset = (uint32_t)((uint8_t*)node - system->base);
	return (BuddyNode*)(system->base + (offset ^ (1u << k)));
}

static uint32_t buddy_order_for_size(uint32_t size) {
	// Adjust size to sizeof(BuddyNode))

	// CLZ is a single instruction on Cortex-M4
	uint32_t k = 31u - (uint32_t)__builtin_clz(size);
	if ((1u << k) < size) k++;

	while ((1u << k) < sizeof(BuddyNode)) k++;

	return k;
}

void* buddy_alloc(BuddySystem* system, uint32_t size) {
	// Retrieves k and invokes buddy_system_reservation
	// NOTE: allocates minimum sizeof(BuddyNode) (16) bytes

	if (system == NULL || size == 0) return NULL;
	if (size > (1u << system->m)) return NULL;

	return buddy_system_reservation(system, buddy_order_for_size(size));
}

uint32_t buddy_free(BuddySystem* system, void* ptr, uint32_t size) {
	if (system == NULL || ptr == NULL || size == 0) return 1;
	if (size > (1u << system->m)) return 1;

	return buddy_system_liberation(system, ptr, buddy_order_for_size(size));
}

// Algorithm R (buddy system reservation)
void* buddy_system_reservation(BuddySystem* system, uint32_t k) {
	// NOTE: allocates minimum sizeof(BuddyNode) (16) bytes

	// The algorithm finds and reserves a block of 2**k locations
	/// or reports failure
	// A free block must hold LINKF/LINKB/TAG/KVAL
	if (system == NULL || k > system->m || (1u << k) < sizeof(BuddyNode)) return NULL;

	// R1. [Find block]
	// let j be the smallest integer in the range k <= j <= m
	// for which AVAILF[j] != LOC(AVAIL[j])
	// that is for which the list of available block size 2**j
	// is not empty
	uint32_t j;
	for (j = k; j <= system->m; j++) {
		if (system->list[j].head->LINKF != system->list[j].head) {
			break;
		}
	}
	if (j > system->m) return NULL;   // no block large enough

	// R2. [Remove from list]
	// Set 
	// L = AVAILF[j] = system->list[j].head->LINKF
	// P = LINKF(L) = system->list[j].head->LINKF->LINKF
	// AVAILF[j] = P => system->list[j].head->LINKF = P
	// LINKB(P) = LOC(AVAIL[j]) => P->LINKB = system->list[j].head
	// L->TAG = 0
	BuddyNode* L = system->list[j].head->LINKF;
	buddy_list_remove(L);

	// R4. [Split]
	// Decrease j by 1
	// Set P = L + 2**j
	// P->TAG = 1
	// P->KVAL = j
	// P->LINKF = P->LINKB = AVAIL[j] = system->list[j].head
	// AVAILF[j] = AVAILB[j] = P =>
	// system->list[j].head->LINKF = P
	// system->list[j].head->LINKB = P
	while (j > k) {
		j--;
		BuddyNode* P = buddy_address(system, L, j);
		buddy_list_insert(P, &system->list[j]);
	}

	// R3. [Split required?]
	// if j == k, the algorithm terminates
	// (we have found and reserved an available block starting at address L)

	return L;
}

// Algorithm S (buddy system liberation)
uint32_t buddy_system_liberation(BuddySystem* system, void* L, uint32_t k) {
	// This algorithm returns a block of 2**k locations,
	// starting in address L, to free storage

	if (system == NULL || L == NULL) return 1;
	if (k > system->m || (1u << k) < sizeof(BuddyNode)) return 1;

	uint32_t offset = (uint32_t)((uint8_t*)L - system->base);
	if (offset >= (1u << system->m)) return 2;   // outside the arena
	if (offset & ((1u << k) - 1)) return 3;      // not 2**k aligned

	BuddyNode* block = (BuddyNode*)L;

	// S1. [Is buddy available]
	// If k == m: Go to S3
	while (k < system->m) {
		// Set P = buddy_k(L)
		BuddyNode* P = buddy_address(system, block, k);

		// P->TAG == 0: Go to S3
		if (P->TAG == false) break;   // buddy is reserved

		// if P->TAG == 1 and P->KVAL != k: Go to S3
		if (P->KVAL != k) break;      // buddy is available but split smaller

		// S2. [Combine with buddy]
		// Remove P from AVAIL[k]; the merged pair keeps the lower address
		buddy_list_remove(P);
		if ((uint8_t*)P < (uint8_t*)block) block = P;
		k++;
	}

	// S3. [Put on list]
	// buddy_list_insert sets TAG = 1 and KVAL = k
	return buddy_list_insert(block, &system->list[k]);
}


