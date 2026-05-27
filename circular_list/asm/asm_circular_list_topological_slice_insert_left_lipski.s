.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_topological_slice_insert_left_lipski
	.type asm_circular_list_topological_slice_insert_left_lipski, %function


@ Using Topological Slice
@ Using Loop Unrolling 2
@ Using Lipski Trick (Registers Permutation Identity) mod 2
@ Using Peeling
@ Using Latency Hiding (Instruction Scheduling)
@ Same as insert right with starting to add info from the end

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
@ .equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
.equ CIRCULAR_AVAIL,4
@ .equ CIRCULAR_SIZE, 8

@ Input:
@ R0 CircularList* circular_list
@ R1 nodes to Insert Right

@ Runtime:
@ R0 CircularList* circular_list
@ R1 nodes to Insert Right
@ R2 Avail, P, P->link
@ R3 P
@ R4 max_nodes
@ R6 AVAIL

@ Return 0 or 1

asm_circular_list_topological_slice_insert_left_lipski:
	PUSH {R4-R6, LR}
	CBZ R0, return_error
	CBZ R1, return_error

	LDR R3, [R0, #CIRCULAR_AVAIL]	@ R2 = AVAIL = P
	MOVS R6, R3						@ R6 = AVAIL

	ADDS R4, R1, #1

	TST R1, #1
	BEQ insert_right_loop			@ if R1 is even go to loop

peel_one_node:
	CBZ R3, return_error

	SUBS R5, R4, R1					@ Update INFO
	LDR R2, [R3, #NODE_LINK]		@ P = P->link
	STR R5, [R3, #NODE_INFO]		@ Store Info

	SUBS R1, R1, #1
	CBZ R1, synchronize_insert_right

	MOVS R3, R2

@ Note: Stores same info twice
.balign 4
insert_right_loop:
@ First Part
	CBZ R3, return_error

	SUBS R5, R4, R1					@ Update INFO
	LDR R2, [R3, #NODE_LINK]		@ P = P->link
	STR R5, [R3, #NODE_INFO]		@ Store Info

@ Second Part
	CBZ R2, return_error

	SUBS R5, R4, R1					@ Update INFO
	LDR R3, [R2, #NODE_LINK]		@ P = P->link
	STR R5, [R3, #NODE_INFO]		@ Store Info

	SUBS R1, R1, #2
	BNE insert_right_loop

synchronize_registers:
	MOVS R1, R2
	MOVS R2, R3
	MOVS R3, R1

synchronize_insert_right:
	LDR R4, [R0, #CIRCULAR_PTR]		@ R4 = PTR
	CBZ R4, ptr_is_null

@ ptr_is_not_null:
	LDR R5, [R4, #NODE_LINK]		@ PTR->link
	STR R5, [R3, #NODE_LINK]		@ P->link = PTR->link
	STR R6, [R4, #NODE_LINK]		@ PTR->link = AVAIL
	STR R2, [R0, #CIRCULAR_AVAIL]	@ AVAIL = P->link
	STR R3, [R0, #CIRCULAR_PTR]		@ PTR = P

	B done

ptr_is_null:
	STR R6, [R3, #NODE_LINK]		@ P->link = AVAIL	// last point to first
	STR R3, [R0, #CIRCULAR_PTR]		@ PTR = P
	STR R2, [R0, #CIRCULAR_AVAIL]	@ AVAIL = P->link;

done:
	MOVS R0, #1
	POP {R4-R6, PC}

return_error:
	MOVS R0, #0
	POP {R4-R6, PC}
