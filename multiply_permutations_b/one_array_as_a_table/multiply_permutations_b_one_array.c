#include <stdint.h>
#include <string.h>


// Domain-Specific Constraint Exploitation: Input is strictly lowercase ASCII.
// Mapping 'a'-'z' (97-122) down to 0-25 creates a perfect O(1) and using array as a symbol table.
// Reduces memory footprint from 128 bytes to 26 bytes, fitting entirely in L1 cache.


uint32_t fill_auxiliary_table(char *permutation, uint32_t permutation_length, int8_t *auxiliary_table) {
    int32_t multiplication_result_length = 0;
	uint32_t j = 0;
    char Z = 1;
    char element;

    // char current_element; will crash the algorithm according to signed-unsigned char specific
    int8_t current_element;
    uint32_t element_index;

    // Set -1 as a default value for symbols
    for (int k = 0; k < 26; k++) {
    	auxiliary_table[k] = -1;
    }

    for (int32_t i = permutation_length - 1; i >= 0; i--) {
        element = permutation[i];

        if (element == ')') {
            Z = 1;
            continue;
        }

        if (element == '(') {
            auxiliary_table[j] = Z;
            continue;
        }

        element_index = element-97;
        current_element = auxiliary_table[element_index];

        if (current_element == -1) {
        	current_element = element;
        	multiplication_result_length++;
        }

        auxiliary_table[element_index] = Z;

        if (Z == 1) {
            j = element_index;
        }

        Z = current_element;
    }

    return multiplication_result_length;
}

void parse_multiplication_result_tag_zero(char *permutation, uint32_t permutation_length, int8_t *auxiliary_table, char *multiplication_result, uint32_t multiplication_result_length) {
    char current_char;
    char next_char;
    uint32_t element_index;

    for (uint32_t i = 0; i < permutation_length; i++) {
    	current_char = permutation[i];

    	if (current_char == '(' || current_char == ')') {
    		continue;
    	}

    	element_index = current_char - 97;

        if (auxiliary_table[element_index] == 0){
            continue;
        }

        *multiplication_result++ = '(';

        while (1) {
        	element_index = current_char - 97;
        	next_char = auxiliary_table[element_index];

        	if (next_char == 0) {
        		break;
        	}

            *multiplication_result++ = current_char;
            multiplication_result_length--;

            // tag using 0 instead of MSB with auxiliary_table[element_index] |= 0x80;
            auxiliary_table[element_index] = 0;
            current_char = next_char;

        }

        *multiplication_result++ = ')';
        multiplication_result -= 3 * (*(multiplication_result - 3) == '(');

        if (multiplication_result_length == 0) {
        	*multiplication_result = '\0';
        	return;
        }
    }
}

void parse_multiplication_result_tag_msb(char *permutation, uint32_t permutation_length, int8_t *auxiliary_table, char *multiplication_result, uint32_t multiplication_result_length) {
    char current_char;
    uint32_t element_index;

    for (uint32_t i = 0; i < permutation_length; i++) {
		current_char = permutation[i];

		if (current_char == '(' || current_char == ')') {
			continue;
		}

		element_index = current_char - 97;

		// An empty node is 0. A tagged node has the MSB set (negative in int8_t).
		if (auxiliary_table[element_index] <= 0) {
			continue;
		}

		*multiplication_result++ = '(';

		while (1) {
			element_index = current_char - 97;

			// Break condition: If the entry we are standing on is already
			// tagged (negative) or empty (0), the cycle has closed!
			if (auxiliary_table[element_index] <= 0) {
				break;
			}

			*multiplication_result++ = current_char;
			multiplication_result_length--;

			// 1. Read the next character DIRECTLY into current_char.
			// We can safely overwrite it because element_index remembers where we are!
			current_char = auxiliary_table[element_index];

			// 2. Tag the trail behind us using the MSB.
			// The original value is perfectly preserved in the lower 7 bits.
			auxiliary_table[element_index] |= 0x80;
		}

		*multiplication_result++ = ')';
		multiplication_result -= 3 * (*(multiplication_result - 3) == '(');

		if (multiplication_result_length == 0) {
			*multiplication_result = '\0';
			return;
		}
	}
}

void multiply_permutations_b_one_array(char *permutation, uint32_t permutation_length, char *multiplication_result) {
    int8_t auxiliary_table[26];

    uint32_t multiplication_result_length = fill_auxiliary_table(permutation, permutation_length, auxiliary_table);

    parse_multiplication_result_tag_zero(permutation, permutation_length, auxiliary_table, multiplication_result, multiplication_result_length);

    // slower
    //parse_multiplication_result_tag_msb(permutation, permutation_length, auxiliary_table, multiplication_result, multiplication_result_length);
}
