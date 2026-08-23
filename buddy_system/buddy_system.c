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

// Algorithm R (buddy system reservation)
void* buddy_system_reservation(BuddySystem* system, uint32_t k) {
	// The algorithm finds and reserves a block of 2**k locations
	/// or reports failure
	if (system == NULL || k > system->m) return NULL;

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

