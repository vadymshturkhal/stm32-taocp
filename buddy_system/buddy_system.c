#include <stdint.h>
#include <stddef.h>

#include "buddy_system.h"

extern uint8_t _end;          // first byte after .bss, ALIGN(8) in the .ld
extern uint8_t _stack_limit;  // _estack - 2048, in the STM32G431RBTX_FLASH.ld
static BuddySystem system;
static int is_buddy_ready = 0;

// Prototypes
static uint32_t buddy_system_liberation(void* L, uint32_t k);
static void* buddy_system_reservation(uint32_t k);

#ifdef BUDDY_MALLOC
// Replacing malloc
void* malloc(size_t size) { return buddy_alloc(size); }
void  free(void* ptr)     { buddy_free(ptr); }
#endif

static uint32_t buddy_system_init(BuddySystem* system) {
	if (system == NULL) return 1;

	const uint32_t m = BUDDY_M;
	uint8_t* memory = &_end;

	uint32_t arena_size = 1u << m;
	uint32_t lists_size = (m + 1) * sizeof(BuddyList);
	uint32_t heads_size = (m + 1) * sizeof(BuddyNode);

	// Init m
	system->m = m;

	// The .ld asserts _end <= _stack_limit, so this cannot go negative
	if (arena_size + lists_size + heads_size > (uint32_t)(&_stack_limit - &_end)) return 4;

	BuddyNode* block = (BuddyNode*)memory;
	BuddyList* list = (BuddyList*)((uint8_t*)memory + arena_size);
	BuddyNode* head = (BuddyNode*)((uint8_t*)list + lists_size);

	system->list = list;
	system->base = (uint8_t*)memory;

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
static BuddyNode* buddy_address(BuddyNode* node, uint32_t k) {
	uint32_t offset = (uint32_t)((uint8_t*)node - system.base);
	return (BuddyNode*)(system.base + (offset ^ (1u << k)));
}

static uint32_t buddy_order_for_size(uint32_t size) {
	// A 2**k block yields 2**k - BUDDY_HEADER usable bytes, since TAG stays
	uint32_t need = size + BUDDY_HEADER;

	// CLZ is a single instruction on Cortex-M4
	uint32_t k = 31u - (uint32_t)__builtin_clz(need);
	if ((1u << k) < need) k++;

	while ((1u << k) < sizeof(BuddyNode)) k++;

	return k;
}

void* buddy_alloc(size_t size) {
	// Retrieves k and invokes buddy_system_reservation
	// NOTE: allocates minimum sizeof(BuddyNode) - BUDDY_HEADER, currently (12) bytes
    if (!is_buddy_ready) {
        buddy_system_init(&system);
        is_buddy_ready = 1;
    }

	if (size == 0) return NULL;
	if (size > (1u << system.m) - BUDDY_HEADER) return NULL;

	void* L = buddy_system_reservation(buddy_order_for_size(size));
	if (L == NULL) return NULL;

	// The caller never sees TAG
	return (uint8_t*)L + BUDDY_HEADER;
}

uint32_t buddy_free(void* ptr) {
	// The block carries its own KVAL, so no size is needed
	if (ptr == NULL) return 1;

	BuddyNode* L = (BuddyNode*)((uint8_t*)ptr - BUDDY_HEADER);

	// KVAL must be in the arena before it is read
	uint32_t offset = (uint32_t)((uint8_t*)L - system.base);
	if (offset >= (1u << system.m)) return 2;

	return buddy_system_liberation(L, L->KVAL);
}

// Algorithm R (buddy system reservation)
static void* buddy_system_reservation(uint32_t k) {
	// The algorithm finds and reserves a block of 2**k locations
	// or reports failure
	// A free block must hold LINKF/LINKB/TAG/KVAL
	if (k > system.m || (1u << k) < sizeof(BuddyNode)) return NULL;

	// R1. [Find block]
	// let j be the smallest integer in the range k <= j <= m
	// for which AVAILF[j] != LOC(AVAIL[j])
	// that is for which the list of available block size 2**j
	// is not empty
	uint32_t j;
	for (j = k; j <= system.m; j++) {
		if (system.list[j].head->LINKF != system.list[j].head) {
			break;
		}
	}
	if (j > system.m) return NULL;   // no block large enough

	// R2. [Remove from list]
	// Set 
	// L = AVAILF[j] = system->list[j].head->LINKF
	// P = LINKF(L) = system->list[j].head->LINKF->LINKF
	// AVAILF[j] = P => system->list[j].head->LINKF = P
	// LINKB(P) = LOC(AVAIL[j]) => P->LINKB = system->list[j].head
	BuddyNode* L = system.list[j].head->LINKF;
	buddy_list_remove(L);

	// L->TAG = 0
	L->TAG = 0;

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
		BuddyNode* P = buddy_address(L, j);
		buddy_list_insert(P, &system.list[j]);
	}

	// R3. [Split required?]
	// if j == k, the algorithm terminates
	// (we have found and reserved an available block starting at address L)

	// KVAL is in every block, was not in TAOCP
	L->KVAL = k;

	return L;
}

// Algorithm S (buddy system liberation)
static uint32_t buddy_system_liberation(void* L, uint32_t k) {
	// This algorithm returns a block of 2**k locations,
	// starting in address L, to free storage

	if (L == NULL) return 1;
	if (k > system.m || (1u << k) < sizeof(BuddyNode)) return 1;

	uint32_t offset = (uint32_t)((uint8_t*)L - system.base);
	if (offset >= (1u << system.m)) return 2;   // outside the arena
	if (offset & ((1u << k) - 1)) return 3;      // not 2**k aligned

	// NOTE: not in TAOCP, early return
	// TAG == 1 means the block is already available: double free
	if (((BuddyNode*)L)->TAG) return 4;

	BuddyNode* block = (BuddyNode*)L;

	// S1. [Is buddy available]
	// If k == m: Go to S3
	while (k < system.m) {
		// Set P = buddy_k(L)
		BuddyNode* P = buddy_address(block, k);

		// P->TAG == 0: Go to S3
		if (P->TAG == 0) break;   // buddy is reserved

		// if P->TAG == 1 and P->KVAL != k: Go to S3
		if (P->KVAL != k) break;      // buddy is available but split smaller

		// S2. [Combine with buddy]
		// Remove P from AVAIL[k]; the merged pair keeps the lower address
		buddy_list_remove(P);
		k++;
		if ((uint8_t*)P < (uint8_t*)block) {
			block = P;
		}
	}

	// S3. [Put on list]
	// buddy_list_insert sets TAG = 1 and KVAL = k
	return buddy_list_insert(block, &system.list[k]);
}
