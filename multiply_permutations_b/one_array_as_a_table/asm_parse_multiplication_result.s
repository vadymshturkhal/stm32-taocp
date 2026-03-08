.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_parse_multiplication_result


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 int8_t *auxiliary_table;
@ R3 char *multiplication_result;
@ R4 uint32_t multiplication_result_length;

@ Runtime:
@ R1 permutation + permutation_length;
@ R6 char current_char;
@ R7 char next_char;
@ R8 uint32_t element_index;


asm_parse_multiplication_result:
	PUSH {R4-R8, LR}

	@ SP Offset = (Number of Registers Pushed) × 4 = 6*4=20
	LDR R4, [SP, #24]	@ get R4 from the stack
	ADD R1, R0, R1

loop_permutation:
	LDRB R6, [R0], #1	@ current_char = permutation[i];

	@ if (current_char == '(' || current_char == ')')
    SUB R12, R6, #40
    CMP R12, #1
    BLS loop_check		@ of R12 is 0 or 1 it's a parenthesis, skip it

	SUB R8, R6, #97 	@ element_index = current_char - 97;
	LDRB R7, [R2, R8]	@ next_char = auxiliary_table[element_index];
	@ if (auxiliary_table[element_index] == 0)

	TST R7, #0x80
    BNE loop_check      @ NEW: Skip if MSB is 1 (we already visited this node!)

	@ *multiplication_result++ = '(';
	MOV R12, '('
	STRB R12, [R3], #1

cycle_loop:
	SUB R8, R6, #97 	@ element_index = current_char - 97;
	LDRB R7, [R2, R8]	@ next_char = auxiliary_table[element_index];

	TST R7, #0x80
	BNE add_right_parenthesis

   	STRB R6, [R3], #1	@ *multiplication_result++ = current_char;
    SUB R4, R4, #1		@ multiplication_result_length--;

    CBZ R4, add_right_parenthesis

    @ tag current_char using MSB and store it
    ORR R6, R6, #0x80
    STRB R6, [R2, R8]

    MOV R6, R7 			@ current_char = next_char;
	B cycle_loop

add_right_parenthesis:
	@ *multiplication_result++ = ')';
	MOV R12, ')'
	STRB R12, [R3], #1

@ Knuth style
singleton_elimination:
	@ multiplication_result -= 3 * (*(multiplication_result - 3) == '(');
	LDRB R12, [R3, #-3]
	CMP R12, '('
	IT EQ
	SUBEQ R3, #3

check_remaining_symbols_length:
	CMP R4, #0
	ITTT EQ
	MOVEQ R12, #0
	STRBEQ R12, [R3], #1
	BEQ done

loop_check:
	CMP R0, R1
	BNE loop_permutation

done:
	MOV R12, #0
    STRB R12, [R3]

	POP {R4-R8, PC}
