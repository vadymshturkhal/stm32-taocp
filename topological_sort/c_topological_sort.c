#include <stdint.h>
#include <stddef.h>
#include "pairs.h"


uint8_t c_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output) {
	// T1 (Initialize)
	TopologicalNode* P;
	uint32_t FRONT;
	uint32_t REAR;

	// Init COUNT
	uint32_t* COUNT = asm_balloc((n+1)*sizeof(uint32_t));
	for (int32_t i = n; i > -1; i--) {
		COUNT[i] = 0;
	}

	// Create QLINK
	uint32_t* QLINK = COUNT;

	// Init TOP
	TopologicalNode** TOP = asm_balloc((n + 1) * sizeof(TopologicalNode*));
	for (int32_t i = n; i > -1; i--) {
		TOP[i] = NULL;
	}

	uint32_t N = n;

	// Init Avail List
	void* AVAIL_LIST = asm_balloc(input_pairs_len * sizeof(TopologicalNode));
	TopologicalNode* Avail = init_avail_list(AVAIL_LIST, input_pairs_len);
	if (Avail == NULL) return 0;

	// T2 (Next relation)
	for (int32_t i = input_pairs_len - 1; i > -1; i--) {
		uint32_t j = input_pairs[i].j;
		uint32_t k = input_pairs[i].k;

		// T3 (Record the relation)
		COUNT[k]++;

		P = Avail;
		if (P == NULL) {
			// overflow
			asm_balloc_free(COUNT);
			return 0;
		}
		Avail = Avail->next;
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

	// Free all memory at once
	asm_balloc_free(COUNT);

	// T8 (End of process)
	if (N == 0) return 1;
	return 0;
};
