#include <stdint.h>
#include <string.h>


// Hacked version
// As a standard ASCII character uses only 7 bits,
// we could use the 8th most significant bit for tagging in-place.
// Tagging (set MSB to 1): 1000 0000 = 0x80 => char | 0x80;
// Clearing (read ignoring the MSB): 0111 1111 = 0x7F => char & 0x7F.


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
char *first_pass_hack(char *permutation, char *parsed_permutation, uint32_t permutation_length) {
	// A1
	// Tag all left parentheses,
	// and replace each right parenthesis
	// by a tagged copy of the element that follows its matching left parenthesis
	char cycle_begin;
	char current_char;
	for (uint32_t i = 0; i < permutation_length; ++i) {
		current_char = permutation[i];

		if (current_char == '(') {
			// tag
			current_char = current_char | 0x80;
			cycle_begin = permutation[i + 1];
		}
		else if (current_char == ')') {
			current_char = cycle_begin;
			// tag
			current_char = current_char | 0x80;
		}
		parsed_permutation[i] = current_char;
	}

	return parsed_permutation;
}

// A2
uint32_t search_untagged_hack(uint8_t *parsed_permutation, uint32_t permutation_length) {
	// Searching from left to right,
	// find the first untagged element of the input and return its index
	// else return 0

	for (uint32_t i = 0; i < permutation_length; ++i) {

		if ((parsed_permutation[i] & 0x80) == 0) {
			return i;
		}
	}

	return 0;
}

uint32_t find_cycle_hack(char *parsed_permutation, uint32_t permutation_length, char* multiplication_result, uint32_t start_index, uint32_t multiplication_result_length) {
	// clear start and current
	char start = parsed_permutation[start_index] & 0x7F;
	char current = parsed_permutation[start_index] & 0x7F;
	char current_char;

	while (1) {
		// A4
		uint32_t i = start_index;
		while (i < permutation_length-1) {
			// clear current_char
			current_char = parsed_permutation[i] & 0x7F;

			// uncomment for getting + 500 cycles for permutation = "(acfg)(bcd)(aed)(fade)(bgfae)"
//			current_char = parsed_permutation[i];
//			if ((current_char & 0x80) != 0) {
//				current_char = current_char & 0x7F;
//			}

			if (current_char == current) {
				// tag
				parsed_permutation[i] = parsed_permutation[i] | 0x80;

				// A3
				// clear current
				current = parsed_permutation[i + 1] & 0x7F;

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

void multiply_permutations_a_hack(char *permutation, uint32_t permutation_length, char *multiplication_result) {
	// permutation = "(acf)(bd)(abd)(ef)";
	// output = "(acefb)(d)"
	// volatile size_t len = strlen(permutation);

	// Init
	uint32_t multiplication_result_length = 0;
	char start;

	// A1
	// Declare parsed_permutation as an array, so the compiler copies it into SRAM (RAM)
	// for granting read/write access to the bits;
	char parsed_permutation[permutation_length];
	first_pass_hack(permutation, parsed_permutation, permutation_length);

	while (1) {
		// A2
		uint32_t start_index = search_untagged_hack(parsed_permutation, permutation_length);

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

		// tag
		// parsed_permutation[start_index] = parsed_permutation[start_index] | 0x80;
		parsed_permutation[start_index] |= 0x80;

		// A3, A4, A5, A6
		multiplication_result_length = find_cycle_hack(parsed_permutation, permutation_length, multiplication_result, start_index, multiplication_result_length);
	}

	// free
	free(parsed_permutation);
}






















