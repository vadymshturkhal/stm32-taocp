.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_topological_slice_insert_right
	.type asm_circular_list_topological_slice_insert_right, %function


@ Using Topological Slice


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
@ R6 AVAIL

@ Return 0 or 1

asm_circular_list_topological_slice_insert_right:
	PUSH {R4-R6, LR}
	CBZ R0, return_error
	CBZ R1, return_error

	LDR R2, [R0, #CIRCULAR_AVAIL]	@ R2 = AVAIL = P
	MOVS R6, R2						@ R6 = AVAIL

.balign 4
insert_right_loop:
	CBZ R2, return_error

	MOVS R3, R2						@ Update Prev P
	STR R1, [R2, #NODE_INFO]		@ Store Info
	LDR R2, [R2, #NODE_LINK]		@ P = P->link

	SUBS R1, R1, #1
	BNE insert_right_loop

synchronize_insert_right:
	LDR R4, [R0, #CIRCULAR_PTR]		@ R4 = PTR
	CBZ R4, ptr_is_null

@ ptr_is_not_null:
	LDR R5, [R4, #NODE_LINK]		@ PTR->link
	STR R5, [R3, #NODE_LINK]		@ P->link = PTR->link
	STR R1, [R4, #NODE_LINK]		@ PTR->link = AVAIL
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
