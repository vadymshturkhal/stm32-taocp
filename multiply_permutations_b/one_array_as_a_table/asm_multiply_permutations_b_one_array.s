.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_multiply_permutations_b_one_array


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 char *multiplication_result;

asm_multiply_permutations_b_one_array:
	PUSH {R4-R6, LR}

    @ allocate 8 bytes for fith argument and 32 bytes for auxiliary_table;
    SUB SP, SP, #40

    @ save inputs
    MOV R4, R0
    MOV R5, R1
    MOV R6, R2

    @ uint32_t multiplication_result_length = asm_fill_auxiliary_table1(permutation, permutation_length, auxiliary_table);
    @ R0 and R1 already contain appropriate values;
	ADD R2, SP, #8		@ auxiliary_table starts at SP + 8;
	BL asm_fill_auxiliary_table2

	@ now R0 contains multiplication_result_length;
	@ store it to the bottom of the stack;
	STR R0, [SP, #0]

	@asm_parse_multiplication_result2(permutation, permutation_length, auxiliary_table, multiplication_result, multiplication_result_length);
	MOV R0, R4
	MOV R1, R5
	ADD R2, SP, #8
	MOV R3, R6

	BL asm_parse_multiplication_result2

done:
	ADD SP, SP, #40
	POP {R4-R6, PC}
