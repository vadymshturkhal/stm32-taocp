.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_multiply_permutations_b_parallel_arrays


@ R0 char *permutation
@ R1 uint32_t permutation_length
@ R2 char *multiplication_result
asm_multiply_permutations_b_parallel_arrays:
	PUSH {R4-R6, LR}

	@ save stack pointer and parameters
	MOV R4, SP
	MOVS R5, R1
	MOVS R6, R2

	@ B1
	@ reserve space on the stack
	@ for two arrays space = permutation_length * 2
	LSLS R2, R1, #1

	@ round space bytes
	ADDS R2, R2, #7
	@ Bit Clear
	BIC R2, R2, #7

	@ allocate the memory on the stack
	SUBS SP, SP, R2

	@ B2, B3, B4
	@ uint32_t i =  asm_get_auxiliary_table4(permutation, permutation_length, original_order, auxiliary_table);

	@ R0 already = permutation
	@ R1 already = permutation_length
	MOV R2, SP
	ADDS R3, SP, R5
	BL asm_get_auxiliary_table5

	@ R0 contains i now

	@ Parsing result
	@ asm_get_multiplication_result(original_order, auxiliary_table, i, multiplication_result);

	MOVS R2, R0
	MOV R0, SP
	ADDS R1, SP, R5
	MOVS R3, R6

	BL asm_get_multiplication_result1

done:
	MOV SP, R4
	POP {R4-R6, PC}
