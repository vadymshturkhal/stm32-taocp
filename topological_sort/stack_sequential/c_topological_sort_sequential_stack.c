#include <stdint.h>
#include <stddef.h>
#include "pairs.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint8_t c_algorithm_t_stack_sequential(uint32_t n, Pair* input_pairs, uint32_t input_pairs_len, uint32_t* output) {
	// T1 (Initialize)

	// Optimization 1: Unified Memory Allocation
	const uint32_t count_size = (n + 1) * sizeof(uint32_t);
	const uint32_t top_size = (n + 1) * sizeof(TopologicalNode*);
	const uint32_t nodes_size = input_pairs_len * sizeof(TopologicalNode);

	uint8_t* master_memory = (uint8_t*)asm_balloc(count_size + top_size + nodes_size);
	if (master_memory == NULL) return 0;

	uint32_t* restrict COUNT = (uint32_t*)master_memory;
	uint32_t* restrict SLINK = COUNT;
	TopologicalNode** restrict TOP = (TopologicalNode**)(master_memory + count_size);
	TopologicalNode* restrict nodes_array = (TopologicalNode*)(master_memory + count_size + top_size);

	// Init COUNT and TOP
	for (int32_t i = n; i > -1; i--) {
		COUNT[i] = 0;
		TOP[i] = NULL;
	}

	uint32_t N = n;
	uint32_t node_alloc_idx = 0;

	// T2 (Next relation)
	for (int32_t i = input_pairs_len - 1; i > -1; i--) {
		uint32_t j = input_pairs[i].j;
		uint32_t k = input_pairs[i].k;

		// T3 (Record the relation)
		COUNT[k]++;

		// Bounds check to prevent buffer overflow
		if (node_alloc_idx >= input_pairs_len) {
			goto exceptions;
		}

		// Grab the next contiguous node and increment the index
		TopologicalNode* P = &nodes_array[node_alloc_idx++];

		P->succ = k;
		P->next = TOP[j];
		TOP[j] = P;
	}

	// T4 (Scan for zeros)
	uint32_t STACK_TOP = 0;
	for (uint32_t k = n; k > 0; k--) {
		if (COUNT[k] == 0) {
			SLINK[k] = STACK_TOP; // Push current node onto stack
			STACK_TOP = k;        // Update top
		}
	}

	// T5-T6 (Process Graph)
	uint32_t queue_output_counter = 0;
	while (STACK_TOP != 0) {
		// T5 (Output top of a stack)
		output[queue_output_counter++] = STACK_TOP; // Output
		N--;

		TopologicalNode* P = TOP[STACK_TOP];
		STACK_TOP = SLINK[STACK_TOP]; // POP from stack

		// T6 (Erase relations)
		while (P != NULL) {
			// COUNT[P->succ] -= 1;

			// Optimization 2: Pre-Decrement Flag Hijack with --COUNT[P->succ] == 0
			// This forces the compiler to use 'SUBS' and check the APSR
			// Zero Flag simultaneously, eliminating a 'CMP' instruction
			if (--COUNT[P->succ] == 0) {
				SLINK[P->succ] = STACK_TOP;		// Push
				STACK_TOP = P->succ;
			}
			P = P->next;
		}
	}

	// T7 (End of process)
	uint8_t topological_status = (N == 0) ? 1 : 0;
//	uint8_t topological_status = (STACK_TOP == 0) ? 1 : 0;	// slightly faster but wrong

cleanup:
	// Free all memory at once
	asm_balloc_free(master_memory);
	return topological_status;

exceptions:
	// Free all memory at once
	asm_balloc_free(master_memory);
	return 0;
};
