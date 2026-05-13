.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_circular_list_insert_right
	.type asm_circular_list_insert_right, %function

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
@ .equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
.equ CIRCULAR_AVAIL,4
@ .equ CIRCULAR_SIZE, 8

@ Input:
@ R0 CircularList* circular_list
@ R1 uint32_t info

@ Runtime:
@ R0 CircularList* circular_list
@ R1 circular_list->ptr
@ R2 circular_list->ptr->link
@ R3
@ R4 circular_list


@ Return 0 or 1

asm_circular_list_insert_right:
	PUSH {R4, LR}
	MOVS R4, R0						@ R4 = circular_list

	BL asm_circular_list_insert_left
	CBZ R0, done

	LDR R1, [R4, #CIRCULAR_PTR]		@ R1 = circular_list->ptr
	LDR R2, [R1, #NODE_LINK]		@ R2 = circular_list->ptr->link
	STR R2, [R4, #CIRCULAR_PTR]		@ circular_list->ptr = circular_list->ptr->link;

@ R0 is 0 if false, else 1 due to asm_circular_list_insert_left return
done:
	POP {R4, PC}
