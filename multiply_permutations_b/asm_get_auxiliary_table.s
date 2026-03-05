.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_auxiliary_table

@ Input:
@ R0 char *permutation
@ R1 uint32_t permutation_length
@ R2 char *original_order
@ R3 char *auxiliary_table


@ Runtime:
@ R0 int32_t original_index;
@ R1 char current_element;

@ R4 char *permutation + permutation_length - 1; reduce and compare with R11
@ R5 char *original_order;
@ R6 char *auxiliary_table;
@ R7 char element;
@ R8 uint32_t i = 0; i is the length of auxiliary_table and original_order;
@ R9 uint32_t j;
@ R10 char Z;
@ R11 char *permutation;

asm_get_auxiliary_table:
	PUSH {R4-R11, LR}

	@ init
	ADD R4, R0, R1
	SUB R4, R4, #1

	MOV R5, R2
	MOV R6, R3
	MOV R8, #0
	MOV R9, #0
	MOV R10, R1
	MOV R11, R0

@ B2
@ decrease R1 and compare to 0
examine_the_next_element:
	CMP R4, R11
    BLO done

	@ element = permutation[index];
	LDRB R7, [R4], #-1

	@ If the element is a ')': set Z = 1 and repeat step B2;
	CMP R7, ')'
	ITT EQ  @ next two instructions if equal
	MOVEQ R10, #1
	BEQ examine_the_next_element

	@ B4
	@ if (element == '('): auxiliary_table[j] = Z; continue;
	CMP R7, '('
	ITT EQ  @ next two instructions if equal
	STRBEQ R10, [R6, R9]
	BEQ examine_the_next_element

	@ find the original index of element
	@ original_index = asm_find_the_original_index2(original_order, i, element);
	MOV R0, R5
	MOV R1, R8
	MOV R2, R7

	BL asm_find_the_original_index2

	@ now R0 = original_index
	CMP R0, #-1
	BEQ add_element_to_arrays

perform_swap:
	@ B3 (change T[i])
	LDRB R1, [R6, R0]	@ current_element = auxiliary_table[original_index];
	STRB R10, [R6, R0]	@ auxiliary_table[original_index] = Z;
	MOV R10, R1			@ Z = current_element;

	@ If T[i] == 1: j = i;
	CMP R10, #1
	IT EQ
	MOVEQ R9, R0		@ j = original_index

	B examine_the_next_element

done:
	MOV R0, R8
	POP {R4-R11, PC}

add_element_to_arrays:
	STRB R7, [R5, R8]	@ original_order[i] = element;
	STRB R7, [R6, R8]	@ auxiliary_table[i] = element;
	MOV R0, R8			@ original_index = i
	ADD R8, R8, #1		@ i += 1

	B perform_swap

