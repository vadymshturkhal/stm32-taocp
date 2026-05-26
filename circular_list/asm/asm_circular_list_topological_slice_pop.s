.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_topological_slice_pop
	.type asm_circular_list_topological_slice_pop, %function


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
@ R1 nodes to Pop

@ Runtime:
@ R0 CircularList* circular_list
@ R1 nodes to Pop
@ R2 PTR
@ R3 P
@ R4 Head
@ R5 info
@ R6 tail

@ Return: 0 if error, else 1

asm_circular_list_topological_slice_pop:
	PUSH {R4-R6, LR}
	CBZ R0, return_error
	CBZ R1, return_error

	LDR R2, [R0, #CIRCULAR_PTR]	@ R2 = PTR
	CBZ R2, return_error

	LDR R3, [R2, #NODE_LINK]	@ R3 = P
	MOVS R4, R3					@ R4 = Head

.balign 4
pop_loop:
	LDR R5, [R3, #NODE_INFO]	@ R5 = P->info
	MOV R6, R3					@ R6 = Tail

	@ we can use MOVSEQ R1, #0 as GNU Assembler (gas) automatically promoted
	@ it to a 32-bit Thumb-2 instruction (movseq.w)
	CMP R2, R3
	ITTE EQ					@ if (circular_list->ptr == P)
	MOVEQ R2, #0				@ R2 = NULL
	MOVEQ R1, #1				@ Reset Counter for break
	LDRNE R3, [R3, #NODE_LINK]	@ P = P->link

	SUBS R1, R1, #1
	BNE pop_loop

update_ptr_link:
	CBZ R2, synchronize_pop
	STR R3, [R2, #NODE_LINK]	@ PRT->link = P->link, if slice a part of the list

synchronize_pop:
	LDR R5, [R0, #CIRCULAR_AVAIL]	@ R5 = circular_list->avail
	STR R5, [R6, #NODE_LINK]		@ Tail->link = AVAIL
	STR R4, [R0, #CIRCULAR_AVAIL]	@ AVAIL = Head
	STR R2, [R0, #CIRCULAR_PTR]		@ Update PTR

done:
	MOVS R0, #1
	POP {R4-R6, PC}

return_error:
	MOVS R0, #0
	POP {R4-R6, PC}
