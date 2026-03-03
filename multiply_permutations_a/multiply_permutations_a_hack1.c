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

void multiply_permutations_a_hack_asm(char *permutation, uint32_t permutation_length, char *multiplication_result) {
	// permutation = "(acf)(bd)(abd)(ef)";
	// output = "(acefb)(d)"
	// volatile size_t len = strlen(permutation);

	// Init
	uint32_t multiplication_result_length = 0;
	char start;

	// A1
	// Declare parsed_permutation as an array, so the compiler copies it into SRAM (RAM)
	// for granting read/write access to the bits;

	// Because I declared char parsed_permutation[permutation_length];
	// with a variable length, GCC dynamically allocated it directly on the stack!
	char parsed_permutation[permutation_length];
	first_pass_hack(permutation, parsed_permutation, permutation_length);
//	first_pass_hack_asm(permutation, parsed_permutation, permutation_length);
//	first_pass_hack_asm1(permutation, parsed_permutation, permutation_length);
//	first_pass_hack_asm2(permutation, parsed_permutation, permutation_length);

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
		// cycles = 3167
		// multiplication_result_length = asm_find_cycle_hack(parsed_permutation, permutation_length, multiplication_result, start_index, multiplication_result_length);
		// cycles = 3083
		// multiplication_result_length = asm_find_cycle_hack1(parsed_permutation, permutation_length, multiplication_result, start_index, multiplication_result_length);
		// cycles = 3083
		multiplication_result_length = asm_find_cycle_hack2(parsed_permutation, permutation_length, multiplication_result, start_index, multiplication_result_length);
	}

	// free
	free(parsed_permutation);
}

