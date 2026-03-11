.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_find_cycle_hack4


@ Using optimal-sequential write (auto-incrementing)
@ Use multiplication_result in place

@ Input:
@ R0 parsed_permutation addres and i;
@ R1 permutation_length;
@ R2 multiplication_result;
@ R3 start_index;

@ Runtime:
@ R0 parsed_permutation addres and i;
@ R1 permutation_length;
@ R2 multiplication_result;
@ R3 parsed_permutation addres + start_index;
@ R4 parsed_permutation addres + permutation_length-1;
@ R5 start;
@ R6 current;
@ R7 current_char;

@ Return:
@ R2 multiplication_result;
asm_find_cycle_hack4:
	PUSH {R4-R7, LR}

	@ char start = parsed_permutation[start_index] & 0x7F;
	LDRB R5, [R0, R3] @ parsed_permutation[start_index]
	AND  R5, R5, 0x7F @ clear

	@ current = start
	MOV R6, R5

	@ R8 = permutation_length-1 + parsed_permutation addres
	SUBS R4, R1, #1
	ADDS R4, R0, R4

	@ Start address
	ADDS R3, R0, R3

@ A4
init_i:
	MOVS R0, R3			@ R0 = start address

.balign 4
comparing_loop:
	CMP R0, R4
	BHS compare_current_and_start

	LDRB R7, [R0], #1	@ current_char = parsed_permutation[i];

	@ Get rid of the tag with AND, UBFX or BIC
	@ AND R7, R7, 0x7F

	@ Unsigned Bit Field Extract same as AND
	@ From right to left
	@ UBFX R7, R7, #0, #7

	BIC R7, R7, #0x80	@ Clear bit 7 (0x80) only

	CMP R7, R6
	BNE comparing_loop

set_new_current:
	ORR R7, R7, 0x80	@ tag
	STRB R7, [R0, #-1]
	LDRB R6, [R0], #1	@ current = parsed_permutation[i + 1] & 0x7F;
	BIC R6, R6, #0x80	@ clear tag

	B comparing_loop

compare_current_and_start:
	CMP R6, R5
	BEQ found_cycle

@ A5
place_char_to_result:
	STRB R6, [R2], #1
	B init_i

@ A6
found_cycle:
	MOV R7, #0x29		@ ASCII ')'
	STRB R7, [R2], #1	@ multiplication_result[multiplication_result_length] = ')';

	@ Knuth singleton elimination
	LDRB R7, [R2, #-3]
	CMP	R7, #0x28		@ ASCII '('
	IT EQ
	SUBEQ R2, R2, #3

done:
	MOV R0, R2
	POP {R4-R7, PC}
