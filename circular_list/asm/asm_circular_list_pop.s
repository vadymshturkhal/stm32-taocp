.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_pop
	.type asm_circular_list_pop, %function

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
@ .equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
.equ CIRCULAR_AVAIL,4
@ .equ CIRCULAR_SIZE, 8

@ Input:
@ R0 bool* pop_is_success
@ R1 CircularList* circular_list

@ Runtime:
@ R0 bool* pop_is_success
@ R1 bool* pop_is_success, 0 or P->link, circular_list->avail
@ R2 circular_list->ptr
@ R3 P

@ Return:
@ pop left
@ return 0 if Underflow, else P->info
@ input pop_is_success flag must always be true

.balign 4
asm_circular_list_pop:
	LDR R2, [R1, #CIRCULAR_PTR]
	CBZ R2, underflow

	LDR R3, [R2, #NODE_LINK]	@ R3 = P

	@ we can use MOVSEQ R1, #0 as GNU Assembler (gas) automatically promoted
	@ it to a 32-bit Thumb-2 instruction (movseq.w).
	CMP R2, R3
	ITTEE EQ					@ if (circular_list->ptr == P)
	MOVEQ R0, #0				@ R0 = NULL
	STREQ R0, [R1]				@ circular_list->ptr = NULL
	LDRNE R0, [R3, #NODE_LINK]	@ R0 = P->link
	STRNE R0, [R2, #NODE_LINK]	@ circular_list->ptr->link = P->link

	LDR R2, [R1, #CIRCULAR_AVAIL]	@ R2 = circular_list->avail
	LDR R0, [R3, #NODE_INFO]		@ R0 = P->info
	STR R2, [R3, #NODE_LINK]		@ P->link = circular_list->avail
	STR R3, [R1, #CIRCULAR_AVAIL]	@ circular_list->avail = P

done:
	BX LR

underflow:
	MOVS R1, #0
	STRB R1, [R0]		@ *pop_is_success = false
	MOVS R0, #0			@ return 0
	BX LR
