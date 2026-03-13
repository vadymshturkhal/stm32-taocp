.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_multiplication_result


@ Input:
@ R0 char *original_order;
@ R1 char *auxiliary_table;
@ R2 uint32_t i;
@ R3 char *multiplication_result;

@ Runtime:
@ R0 untagged_index;
@ R1 current_char;
@ R4 original_order;
@ R5 auxiliary_table;
@ R6 i;
@ R7 multiplication_result;
asm_get_multiplication_result:
	PUSH {R4-R7, LR}

	MOV R4, R0
	MOV R5, R1
	MOV R6, R2
	MOV R7, R3

main_loop:
	@ asm_get_first_untagged(original_order, i);
	MOV R0, R4
	MOV R1, R6
	BL asm_get_first_untagged

	@ R1 contains untagged_index

	@ CMP R0, #-1			@ if (untagged_index == -1)
	@ ITTT EQ
	@ MOVEQ R1, #0		@ store '\0'
	@ STRBEQ R1, [R7], #1 @ *multiplication_result++ = '\0';
	@ BEQ done

	CMP R0, #-1			@ if (untagged_index == -1)
	BEQ pre_done

add_left_parenthesis:
	MOV R1, '('
	STRB R1, [R7], #1	@ *multiplication_result++ = '(';

add_inner_cycle:
	LDRB R1, [R4, R0]	@ current_char = original_order[untagged_index];

	@ tag original_order[untagged_index];
	ORR R2, R1, 0x80
	STRB R2, [R4, R0]

	@ add current_char to multiplication_result;
	STRB R1, [R7], #1

	LDRB R1, [R5, R0]	@ current_char = auxiliary_table[untagged_index]
	MOV R0, #-1			@ untagged_index
	MOV R3, R4			@ original_order
	ADD R12, R4, R6		@ original_order + i

find_the_current_char_index_loop:
	LDRB R2, [R3], #1
	CMP R1, R2			@ if (current_char == original_order[k])
	BEQ found_match

	CMP R3, R12
	BLO find_the_current_char_index_loop

add_right_parenthesis:
	MOV R1, ')'
	STRB R1, [R7], #1	@ *multiplication_result++ = '(';

@ Knuth style singleton elimination
singleton_elimination:
	@ multiplication_result -= 3 * (*(multiplication_result - 3) == '(');
	LDRB R1, [R7, #-3]
	CMP R1, '('
	IT EQ
	SUBEQ R7, #3
	B main_loop

found_match:
	SUB R0, R3, R4
	SUB R0, R0, #1
	B add_inner_cycle

pre_done:
	MOV R1, #0		@ store '\0'
	STRB R1, [R7], #1 @ *multiplication_result++ = '\0';

done:
	POP {R4-R7, PC}
