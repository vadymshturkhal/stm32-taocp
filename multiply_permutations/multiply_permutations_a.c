#include <stdint.h>
#include <string.h>


// Input example: (acf)(bd)(abd)(ef)
// Output: (acefb)


// Algorithm A

// A1
// Tag all left parentheses, and replace each right parenthesis
	// by a tagged copy of the element that follows its matching left parenthesis

// A2
// If all chars are tagged: return output string;
// Searching from left to right, find the first untagged element of the input.
	// Set START equal to it;
	// Add a left parenthesis to the output;
	// Add the element to the output;

// A3
// CURRENT = next element;

// A4
// loop to the right until the end of the string;
	// if found CURRENT: tag it and go to A3

// A5 CURRENT == START?
// If CURRENT != START:
	// add CURRENT to the output;
	// go to A4;

// A6 Complete cycle has been found
	// Add a right parenthesis to the output;
	// go to step A2;


// Overall:
	// input string;
	// temp output string;
	// tag/untag array or hash table;
	// START;
	// CURRENT;

// A1
char *first_pass(char *permutation, uint8_t *tagged, uint32_t permutation_length) {
	// A1
	// Tag all left parentheses,
	// and replace each right parenthesis
	// by a tagged copy of the element that follows its matching left parenthesis
	char *parsed_permutation = (char *)malloc(permutation_length * sizeof(uint8_t));
	char cycle_begin;
	char current_char;
	for (uint32_t i = 0; i < permutation_length; ++i) {
		current_char = permutation[i];

		if (current_char == '(') {
			tagged[i] = 1;
			cycle_begin = permutation[i + 1];
		}
		else if (current_char == ')') {
			tagged[i] = 1;
			current_char = cycle_begin;
		}
		parsed_permutation[i] = current_char;
	}

	return parsed_permutation;
}

// A2
uint32_t search_untagged(uint8_t *tagged, uint32_t permutation_length) {
	// Searching from left to right,
	// find the first untagged element of the input and return its index
	// else return 0

	for (uint32_t i = 0; i < permutation_length; ++i) {
		if (!tagged[i]) {
			return i;
		}
	}

	return 0;
}

uint32_t find_cycle(char *parsed_permutation, uint32_t permutation_length, char *tagged, char* multiplication_result, uint32_t start_index, uint32_t multiplication_result_length) {
	char start = parsed_permutation[start_index];
	char current = parsed_permutation[start_index];
	char current_char;

	while (1) {
		// A4
		uint32_t i = start_index;
		while (i < permutation_length-1) {
			current_char = parsed_permutation[i];

			if (current_char == current) {
				tagged[i] = 1;

				// A3
				current = parsed_permutation[i + 1];
				i++;
			}

			i++;
		}

		// A6
		if (current == start) {
			multiplication_result[multiplication_result_length] = ')';
			multiplication_result_length++;
			return multiplication_result_length;
		}

		// A5
		multiplication_result[multiplication_result_length] = current;
		multiplication_result_length++;
	}
}

void multiply_permutations_a(char *permutation, uint32_t permutation_length, char *multiplication_result) {
	// permutation = "(acf)(bd)(abd)(ef)";
	// output = "(acefb)(d)"
	// volatile size_t len = strlen(permutation);

	// Init
	uint32_t multiplication_result_length = 0;
	char start;

	// Reserve space on the heap
	uint8_t *tagged = calloc(permutation_length, sizeof(uint8_t));

	// A1
	char *parsed_permutation = first_pass(permutation, tagged, permutation_length);


	// A2
	// If all chars are tagged: return output string;
	// Searching from left to right, find the first untagged element of the input.
	// Set START equal to it;
	// Add a left parenthesis to the output;
	// Add the element to the output and tag it;

	while (1) {
		// Searching from left to right, find the first untagged element of the input.
		// Set START equal to it;
		uint32_t start_index = search_untagged(tagged, permutation_length);

		// If all chars are tagged: return output string;
		if (start_index == 0) {
			// Add '\0' to make a string
			multiplication_result[multiplication_result_length] = '\0';
			return;
		}

		// Add a left parenthesis to the output;
		multiplication_result[multiplication_result_length] = '(';
		multiplication_result_length++;

		// Add the element to the output and tag it;
		start = parsed_permutation[start_index];
		multiplication_result[multiplication_result_length] = start;
		multiplication_result_length++;
		tagged[start_index] = 1;


		// A3, A4, A5, A6
		multiplication_result_length = find_cycle(parsed_permutation, permutation_length, tagged, multiplication_result, start_index, multiplication_result_length);
	}

	// free
	free(tagged);
	free(parsed_permutation);
}

