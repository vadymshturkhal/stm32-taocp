.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_find_cycle_hack1


@ Using optimal-sequential write (auto-incrementing)

@ Signature
@ uint32_t find_cycle_hack(char *parsed_permutation,
@							uint32_t permutation_length,
@							char* multiplication_result,
@							uint32_t start_index,
@							uint32_t multiplication_result_length)

@ Registers:
@ R0 parsed_permutation addres and i;
@ R1 permutation_length;
@ R2 multiplication_result;
@ R3 start_index;
@ R4 multiplication_result_length;
@ R5 start;
@ R6 current;
@ R7 parsed_permutation addres + start_index;
@ R8 permutation_length-1;
@ R9 current_char

@ Return:
@ R4;

@ char *permutation = "(acf)(bd)(abd)(ef)";
asm_find_cycle_hack1:
	PUSH {R4-R9, LR}

	@ Retrieve fifth argument from the Stack
	@ We pushed 7 registers * 4 bytes = 24 bytes,
	@ and multiplication_result_length now at SP + 28
	LDR R4, [SP, #28]

	@ char start = parsed_permutation[start_index] & 0x7F;
	LDRB R5, [R0, R3] @ parsed_permutation[start_index]
	AND  R5, R5, 0x7F @ clear

	@ current = start
	MOV R6, R5

	@ R8 = permutation_length-1
	SUB R8, R1, #1
	ADD R8, R0, R8

	@ Start address
	MOV R7, R0
	ADD R7, R7, R3

@ A4
init_i:
	@ R0 = start address
	@ x MOV R7, R3
	MOV R0, R7

comparing_loop:
	@ x CMP R7, R8
	CMP R0, R8
	BHS compare_current_and_start

	@ current_char = parsed_permutation[i] & 0x7F;
	@ x LDRB R9, [R0, R7]
	LDRB R9, [R0]
	AND R9, R9, 0x7F

	CMP R9, R6
	BEQ set_new_current

	@ increment R0
	ADD R0, R0, #1
	B comparing_loop

set_new_current:
	@ parsed_permutation[i] = parsed_permutation[i] | 0x80;
	@ LDRB R9, [R0, R7]
	@ tag
	ORR R9, R9, 0x80
	@ x STRB R9, [R0, R7]
	STRB R9, [R0], #1
	@ increment i
	@ x ADD R7, R7, #1

	@ current = parsed_permutation[i + 1] & 0x7F;
	@ x LDRB R6, [R0, R7]
	LDRB R6, [R0], #1
	AND R6, R6, 0x7F

	@ increment i
	@ ADD R7, R7, #1

	B comparing_loop

compare_current_and_start:
	CMP R6, R5
	BEQ found_cycle

@ A5
place_char_to_result:
	STRB R6, [R2, R4]
	ADD R4, R4, #1
	B init_i

@ A6
found_cycle:
	MOV R9, #0x29	@ASCII ')'

	@ multiplication_result[multiplication_result_length] = ')';
	STRB R9, [R2, R4]
	ADD R4, R4, #1

	@ Knuth singleton elimination
	@ multiplication_result_length -= 3 * (multiplication_result[multiplication_result_length - 3] == '(');
	SUB R12, R4, #3
	LDRB R9, [R2, R12]
	CMP	R9, #0x28	@ ASCII '('
	IT EQ
	SUBEQ R4, R4, #3

done:
	MOV R0, R4
	POP {R4-R9, PC}
