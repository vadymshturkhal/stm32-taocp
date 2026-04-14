#include <stdint.h>
#include <stddef.h>
#include "pairs.h"

TopologicalNode* init_avail_list(void* memory, uint32_t size) {
	if (size == 0 || memory == NULL) return NULL;

	TopologicalNode* avail = (TopologicalNode*)(memory);
	TopologicalNode* tmp;

	// avail->info = size;
	avail->next = NULL;
	size--;

	while (size > 0) {
		tmp = avail+1;
		// tmp->info = size;
		tmp->next = avail;

		avail = tmp;
		size--;
	}

	return avail;
}
