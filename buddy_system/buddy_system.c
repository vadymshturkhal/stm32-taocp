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

	// LINKF(0) = LINKB(0) = LOC(AVAIL[m])
	return buddy_list_insert(block, &list[m]);
}

// Using normalized memory which starts from
BuddyNode* buddy_address(BuddySystem* system, BuddyNode* node, uint32_t k) {
	uint32_t offset = (uint32_t)((uint8_t*)node - system->base);
	return (BuddyNode*)(system->base + (offset ^ (1u << k)));
}
