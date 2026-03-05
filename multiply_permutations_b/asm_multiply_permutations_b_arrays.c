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


// Prototypes
uint32_t asm_get_first_untagged(char *original_order, uint32_t original_order_length);
uint32_t asm_get_auxiliary_table4(char *permutation, uint32_t permutation_length, char *original_order, char *auxiliary_table);
void asm_get_multiplication_result(char *original_order, char *auxiliary_table, uint32_t i, char *multiplication_result);

void asm_c_multiply_permutations_b(char *permutation, uint32_t permutation_length, char *multiplication_result) {
	// B1
	// Reserve space on the stack
	char original_order[permutation_length];
	char auxiliary_table[permutation_length];

	// B2, B3, B4
//	uint32_t i = asm_get_auxiliary_table(permutation, permutation_length, original_order, auxiliary_table);
//	uint32_t i = asm_get_auxiliary_table1(permutation, permutation_length, original_order, auxiliary_table);
//	uint32_t i = asm_get_auxiliary_table2(permutation, permutation_length, original_order, auxiliary_table);
//	uint32_t i = asm_get_auxiliary_table3(permutation, permutation_length, original_order, auxiliary_table);
	uint32_t i = asm_get_auxiliary_table4(permutation, permutation_length, original_order, auxiliary_table);

//	asm_c_get_multiplication_result(original_order, auxiliary_table, i, multiplication_result);
	asm_get_multiplication_result(original_order, auxiliary_table, i, multiplication_result);
}
