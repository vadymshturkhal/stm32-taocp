#include <stdint.h>
#include <string.h>


// Input example: (acf)(bd)(abd)(ef)
// Output: (acefb)


// Algorithm B

// This algorithm accomplishes the same result as Algorithm A;
// Assume that 	the elements permuted are named [x_0, x_1,...,x_(n-1)];
// We use an auxiliary table T[0], T[1],...,T[n-1];
// Upon termination of this algorithm, x_i goes to x_j under the imput
// permutation if and only if T[i] = j;

// B1 (init):
// Set T[k] <- k for k in range n;
// Prepare to scan the input from right to left;

// B2 (next element):
// Examine the next element of input right to left;
// If the input has been exhausted, the algorithm terminates;
// If the element is a ')': set Z = 0 and repeat step B2;
// If it is a '(', go to B4;
// In other case element is x_i for some i;
// Go to B3;

// B3 (change T[i]):
// Exchange Z, T[i];
// If T[i] == 0: j = i;
// Go to B2;

// B4 (change T[j]):
// Set T[j] = Z;

uint32_t get_auxiliary_table(char *permutation, uint32_t permutation_length, char *original_order, char *auxiliary_table) {
	// i is the length of auxiliary_table and original_order
	uint32_t i = 0;
	uint32_t j;
	int32_t original_index;
	char Z;
	char element;
	char current_element;

	// B2
	// Examine the next element of input right to left;
	for (int32_t index = permutation_length - 1; index >= 0; index--) {
		element = permutation[index];

		// If the element is a ')': set Z = 1 and repeat step B2;
		// Z = 1 for avoiding visual string interruption in the middle with placing Z = 0
		if (element == ')') {
			Z = 1;
			continue;
		}

		// B4
		if (element == '(') {
			auxiliary_table[j] = Z;
			continue;
		}

		original_index = -1;

		// find the original index of element
		for (uint32_t k = 0; k < i; k++) {
			current_element = original_order[k];
			if (current_element == element) {
				original_index = k;
				break;
			}
		}

		if ((original_index == -1)) {
			original_order[i] = element;
			auxiliary_table[i] = element;
			original_index = i;
			i += 1;
		}

		// B3 (change T[i])
		current_element = auxiliary_table[original_index];
		auxiliary_table[original_index] = Z;
		Z = current_element;

		// If T[i] == 1: j = i;
		if (auxiliary_table[original_index] == 1) {
			j = original_index;
		}
	}

	return i;
}

uint32_t get_first_untagged(char *original_order, uint32_t original_order_length) {
	// Searching from left to right,
	// find the first untagged element of the original_order
	// and return its index
	// else return -1

	for (uint32_t i = 0; i < original_order_length; i++) {
		if ((original_order[i] & 0x80) == 0) {
			return i;
		}
	}

	return -1;
}

void get_multiplication_result(char *original_order, char *auxiliary_table, uint32_t i, char *multiplication_result) {
	// using tag
	char current_char;

	while (1) {
		int32_t untagged_index = get_first_untagged(original_order, i);

		if (untagged_index == -1) {
			// add '\0'
			*multiplication_result++ = '\0';
			return;
		}

		// add '('
		*multiplication_result++ = '(';

		// add cycle
		while (untagged_index != -1) {
			// set current_char
			current_char = original_order[untagged_index];

			// tag original_order[untagged_index]
			original_order[untagged_index] |= 0x80;

			// add current_char to multiplication_result;
			*multiplication_result++ = current_char;

			// set next char
			current_char = auxiliary_table[untagged_index];

			untagged_index = -1;
			// find the current_char index
			for (uint32_t k = 0; k < i; k++) {
				if (current_char == original_order[k]){
					untagged_index = k;
					break;
				}
			}
		}

		// add ')'
		*multiplication_result++ = ')';

		// Knuth style singleton elimination
		multiplication_result -= 3 * (*(multiplication_result - 3) == '(');
	}
}

void multiply_permutations_b_parallel_arrays(char *permutation, uint32_t permutation_length, char *multiplication_result) {
	// B1
	// Reserve space on the stack
	char original_order[permutation_length];
	char auxiliary_table[permutation_length];

	// B2, B3, B4
	uint32_t i = get_auxiliary_table(permutation, permutation_length, original_order, auxiliary_table);

	get_multiplication_result(original_order, auxiliary_table, i, multiplication_result);
}
