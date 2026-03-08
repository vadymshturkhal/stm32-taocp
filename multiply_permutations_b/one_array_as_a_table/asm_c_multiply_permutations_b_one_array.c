#include <stdint.h>
#include <string.h>


// Domain-Specific Constraint Exploitation: Input is strictly lowercase ASCII.
// Mapping 'a'-'z' (97-122) down to 0-25 creates a perfect O(1) and using array as a symbol table.
// Reduces memory footprint from 128 bytes to 26 bytes, fitting entirely in L1 cache.


void asm1_multiply_permutations_b_one_array(char *permutation, uint32_t permutation_length, char *multiplication_result) {
    int8_t auxiliary_table[26];

    uint32_t multiplication_result_length = asm_fill_auxiliary_table1(permutation, permutation_length, auxiliary_table);
    // cycles = 926
    asm_parse_multiplication_result2(permutation, permutation_length, auxiliary_table, multiplication_result, multiplication_result_length);
}
