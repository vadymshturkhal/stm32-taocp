#include <stdint.h>
#include <stddef.h>
#include "pairs.h"

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

uint8_t c_algorithm_t_queue_sequential(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output) {
	// T1 (Initialize)
	TopologicalNode* P;
	uint32_t FRONT;
	uint32_t REAR;

	// Init COUNT
	uint32_t* COUNT = asm_balloc((n + 1)*sizeof(uint32_t));
	if (COUNT == NULL) return 0;	// Nothing to free

	for (int32_t i = n; i > -1; i--) {
		COUNT[i] = 0;
	}

	// Create QLINK
	uint32_t* QLINK = COUNT;

	// Init TOP
	TopologicalNode** TOP = asm_balloc((n + 1) * sizeof(TopologicalNode*));
	if (TOP == NULL) goto exceptions;

	for (int32_t i = n; i > -1; i--) {
		TOP[i] = NULL;
	}

	uint32_t N = n;

	// Init Sequential Array
	TopologicalNode* nodes_array = NULL;
	uint32_t node_alloc_idx = 0;

	// In case of 0 edges
	if (input_pairs_len > 0) {
		// Allocate the contiguous block
		nodes_array = (TopologicalNode*)asm_balloc(input_pairs_len * sizeof(TopologicalNode));
		if (nodes_array == NULL) goto exceptions;
	}

	// T2 (Next relation)
	for (int32_t i = input_pairs_len - 1; i > -1; i--) {
		uint32_t j = input_pairs[i].j;
		uint32_t k = input_pairs[i].k;

		// T3 (Record the relation)
		COUNT[k]++;

		// Bounds check (standard practice in C to prevent buffer overflows)
		if (node_alloc_idx >= input_pairs_len) {
			goto exceptions;
		}

		// Grab the next contiguous node and increment the index
		TopologicalNode* P = &nodes_array[node_alloc_idx++];

		P->succ = k;
		P->next = TOP[j];

		TOP[j] = P;
	}

	// T4 (At this point we have completed the input phase)
	REAR = 0;
	QLINK[0] = 0;
	for (uint32_t k = n; k > 0; k--) {
		if (COUNT[k] == 0) {
			QLINK[REAR] = k;
			REAR = k;
		}
	}

	FRONT = QLINK[0];

	uint32_t queue_output_counter = 0;
	while (N != 0) {
		if (FRONT == 0) {
			// Go to T8
			break;
		}

		// T5 (Output front of a queue)
		output[queue_output_counter] = FRONT;

		N--;
		P = TOP[FRONT];

		// T6 (Erase relations)
		while (P != NULL) {
			COUNT[P->succ] -= 1;

			if (COUNT[P->succ] == 0) {
				QLINK[REAR] = P->succ;
				REAR = P->succ;
			}

			P = P->next;
		}

		// T7 (Remove from Queue)
		FRONT = QLINK[FRONT];

		queue_output_counter += 1;

		// Go back to T5
	}

	// T8 (End of process)
	uint8_t topological_status = (N == 0) ? 1 : 0;

cleanup:
	// Free all memory at once
	asm_balloc_free(COUNT);
	return topological_status;

exceptions:
	// Free all memory at once
	asm_balloc_free(COUNT);
	return 0;
};
