.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_multiplication_result1


@ inlined search_first_untagged from asm_get_first_untagged.s;


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

asm_get_multiplication_result1:
	PUSH {R4-R7, LR}

	MOV R4, R0
	MOV R5, R1
	MOV R6, R2
	MOV R7, R3

.balign 4
main_loop:
    MOV R0, R4          @ R0 = original_order cursor
    MOV R1, R6          @ R1 = loop counter (countdown)
    ADD R2, R0, #1		@ Trick for avoiding SUBS R0, R0, #1 in found_untagged

.balign 4
search_first_untagged:
    LDRB R3, [R0], #1   @ Fetch and post-increment

    TST R3, 0x80        @ Check the top bit
    BEQ found_untagged  @ If bit is 0, we found it! Jump out of the loop.

    SUBS R1, R1, #1
    BNE search_first_untagged

    B pre_done			@ jump to the end

.balign 4
found_untagged:
    @ Calculate the exact index
    SUB R0, R0, R2      @ R0 = untagged_index

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
