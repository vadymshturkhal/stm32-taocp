.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_fill_auxiliary_table


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 int8_t *auxiliary_table;

@ Runtime:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 int8_t *auxiliary_table;
@ R3 uint32_t j = 0;
@ R4 char Z = 1;
@ R5 char element;
@ R6 char current_element;
@ R7 uint32_t element_index;
@ R8 int32_t multiplication_result_length = 0;
@ R9 permutation + permutation_length;

@ Return:
@ multiplication_result_length


asm_fill_auxiliary_table:
	PUSH {R4-R9, LR}

	MOV R3, #0	@ j = 0;
	MOV R4, #1	@ Z = 1;
	MOV R8, #0	@ multiplication_result_length = 0;

	MOV R5, R2 @ copy auxiliary_table for set_default_values_loop
	ADD R6, R2, #26
	MOV R7, #-1


@ Set -1 as a default value for auxiliary_table[k]
set_default_values:
	@ Blast 26 bytes of -1 into the array instantly using STRD
	MOV R6, #-1			@ Setup first 32-bit block
	MOV R7, #-1			@ Setup second 32-bit block

	@ Write 8 bytes per clock cycle
	STRD R6, R7, [R2, #0]	@ Bytes 0-7
	STRD R6, R7, [R2, #8]	@ Bytes 8-15
	STRD R6, R7, [R2, #16]	@ Bytes 16-23

	@ Write the final 2 bytes (24-25) with a Half-word store
	STRH R7, [R2, #24]

prepare_to_permutation_loop:
	ADD R9, R0, R1

@ for (int32_t i = permutation_length - 1; i >= 0; i--) {
@ B2
permutation_loop:
	LDRB R5, [R9, #-1]!	@ element = permutation[i];

	@ if the element == ')': set Z = 1 and repeat step B2;
    CMP R5, ')'
    ITT EQ
    MOVEQ R4, #1
    BEQ loop_check

    @ B4
    CMP R5, '('
    ITT EQ
    STRBEQ R4, [R2, R3]
    BEQ loop_check

    SUB R7, R5, #97			@ element_index;
    LDRSB R12, [R2, R7]		@ auxiliary_table[element_index];
    CMP R12, #-1
    ITT EQ
    STRBEQ R5, [R2, R7]		@ auxiliary_table[element_index] = element;
    ADDEQ R8, R8, #1		@ multiplication_result_length++;

	@ B3 (swap)
	LDRB R6, [R2, R7]		@ current_element = auxiliary_table[element_index];
	STRB R4, [R2, R7]
	MOV R12, R4				@ save prev Z
	MOV R4, R6

	CMP R12, #1
	IT EQ
	MOVEQ R3, R7			@ j = element_index

loop_check:
	CMP R0, R9
	BNE permutation_loop

done:
	MOV R0, R8
	POP {R4-R9, PC}

