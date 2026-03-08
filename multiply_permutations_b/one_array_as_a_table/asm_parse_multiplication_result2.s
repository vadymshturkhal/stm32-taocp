.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_parse_multiplication_result2


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 int8_t *auxiliary_table;
@ R3 char *multiplication_result;
@ R4 uint32_t multiplication_result_length;

@ Runtime:
@ R1 permutation + permutation_length;
@ R5 #0;
@ R6 char current_char;
@ R7 char next_char;
@ R12 as tmp;


asm_parse_multiplication_result2:
	PUSH {R4-R7, LR}

	@ SP Offset = (Number of Registers Pushed) × 4 = 6*4=24
	LDR R4, [SP, #20]	@ get R4 from the stack
	ADDS R1, R0, R1
	MOVS R5, #0
	SUBS R2, R2, #97

loop_permutation:
	LDRB R6, [R0], #1	@ current_char = permutation[i];

	@ second place
    @ CMP R6, '('
    @ BEQ loop_permutation
    @ CMP R6, ')'
    @ BEQ loop_permutation

	@ third place
	@ skip parentheses
    @ CMP R6, 'a'
    @ BLO loop_permutation	@ Cold Path for parentheses

	@ first place
    SUBS R7, R6, 'a'
    BMI loop_permutation

	LDRB R7, [R2, R6]	@ next_char = auxiliary_table[element_index];

	@ if (auxiliary_table[element_index] == 0)
    CBZ R7, loop_check

	@ slower than CBZ with branch
    @ CMP R7, #0          @ Back to standard CMP
    @ BEQ loop_permutation @ BEQ can jump backward!

start_cycle:
	@ *multiplication_result++ = '(';
	MOV R12, '('
	STRB R12, [R3], #1


cycle_loop:
   	STRB R6, [R3], #1	@ *multiplication_result++ = current_char;
    SUBS R4, R4, #1		@ multiplication_result_length--;

    STRB R5, [R2, R6]	@ tag current_char using 0
    MOVS R6, R7 			@ current_char = next_char;
	LDRB R7, [R2, R6]	@ next_char = auxiliary_table[element_index];

	CMP R7, #0
	BNE cycle_loop

add_right_parenthesis:
	@ *multiplication_result++ = ')';
	MOVS R6, ')'
	STRB R6, [R3], #1

@ Knuth style
singleton_elimination:
	@ multiplication_result -= 3 * (*(multiplication_result - 3) == '(');
	LDRB R6, [R3, #-3]
	CMP R6, '('
	BNE check_remaining_symbols_length
    SUBS R3, R3, #3     @ Rewind pointer by 3

check_remaining_symbols_length:
	CMP R4, #0
	BEQ all_symbols_processed

loop_check:
	CMP R0, R1
	BNE loop_permutation

	@ SUBS R6, R0, R1
	@ BMI loop_permutation

	@ done
	POP {R4-R7, PC}

all_symbols_processed:
	STRB R5, [R3]

done:
	POP {R4-R7, PC}
