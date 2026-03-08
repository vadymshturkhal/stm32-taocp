.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_fill_auxiliary_table2


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 int8_t *auxiliary_table;

@ Runtime:
@ R0 char *permutation;
@ R1 permutation + permutation_lengt;
@ R2 int8_t *auxiliary_table;
@ R3 uint32_t j = 0;
@ R4 char Z = 1;
@ R5 char element;
@ R6 char current_element;
@ R7 uint32_t element_index;
@ R8 int32_t multiplication_result_length = 0;

@ Return:
@ multiplication_result_length

asm_fill_auxiliary_table2:
	PUSH {R4-R8, LR}

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

	@ Store Register Dual writes 8 bytes per clock cycle
	STRD R6, R7, [R2, #0]	@ Bytes 0-7
	STRD R6, R7, [R2, #8]	@ Bytes 8-15
	STRD R6, R7, [R2, #16]	@ Bytes 16-23

	@ Write the final 2 bytes (24-25) with a Half-word store
	STRH R7, [R2, #24]

prepare_to_permutation_loop:
	SUBS R2, R2, #97
	ADD R1, R0, R1

@ for (int32_t i = permutation_length - 1; i >= 0; i--) {
@ B2
.balign 4
permutation_loop:
	LDRB R5, [R1, #-1]!	@ element = permutation[i];

	@ if the element == ')': set Z = 1 and repeat step B2;
	SUBS R7, R5, #97
    BMI handle_parentheses   @	Cold Path

    LDRSB R7, [R2, R5]		@ auxiliary_table[element_index];
    CMP R7, #-1
    ITT EQ
    MOVEQ R7, R5			@ R12 = current_element;
    ADDEQ R8, R8, #1		@ multiplication_result_length++;

	@ store Z
	STRB R4, [R2, R5]

	@ B3
	@ check if Z == 1
	CMP R4, #1
	IT EQ
	MOVEQ R3, R5			@ j = element_index

	MOV R4, R7				@ Z = current_element
	CMP R0, R1
	BNE permutation_loop
	B done

handle_parentheses:
	@ if the element == ')': set Z = 1 and repeat step B2;
    CMP R5, ')'
    ITE EQ
    MOVEQ R4, #1
    @ B4
    STRBNE R4, [R2, R3]

loop_check:
	CMP R0, R1
	BNE permutation_loop

done:
	MOV R0, R8
	POP {R4-R8, PC}
