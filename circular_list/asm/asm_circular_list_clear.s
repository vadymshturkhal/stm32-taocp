.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_clear
	.type asm_circular_list_clear, %function

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
@ .equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
.equ CIRCULAR_AVAIL,4
@ .equ CIRCULAR_SIZE, 8

@ Input:
@ R0 CircularList* circular_list

@ Return: void
asm_circular_list_clear:
	@ 1
	LDR R1, [R0, #CIRCULAR_PTR]		@ R1 = PTR
	CBZ R1, done

	LDR R2, [R0, #CIRCULAR_AVAIL]	@ R2 = P = circular_list->avail
	LDR R3, [R1, #NODE_LINK]		@ R3 = circular_list->ptr->link

	STR R3, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = circular_list->ptr->link
	STR R2, [R1, #NODE_LINK]		@ circular_list->ptr->link = P

	MOVS R3, #0						@ R3 = NULL
	STR R3, [R0, #CIRCULAR_PTR]		@ R1 = PTR

done:
	BX LR
