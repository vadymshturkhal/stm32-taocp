#include "main.h"
#include <string.h>


void inverse_permutation_in_place_j(uint32_t *permutation, uint32_t permutation_length) {
	// permutation should include 0 at 0 index;

	// J1 (Negate all)
	for (uint32_t i = 0; i < permutation_length; i++){
		permutation[i] = -permutation[i];
	}

	int32_t m = permutation_length - 1;
	int32_t j;
	int32_t i;

	while (m > 0) {
		// J2 (Initialize)
		j = m;

		// J3 (Find negative entry)
		// Set i = permutations[j], if i > 0: j = i and repeat
		i = permutation[j];
		while (i > 0) {
			j = i;
			i = permutation[j];
		}

		// J4 (Invert)
		// Set permutation[j] = permutation[-i], permutation[-i] = m
		permutation[j] = permutation[-i];
		permutation[-i] = m;

		// J5 (Loop on m)
		// Decrese m by 1; if m > 0, go back to J2, terminate otherwise
		m--;
	}
}
