.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_circular_list_insert_left
	.type asm_circular_list_insert_left, %function

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
@ R1 uint32_t info
@ R2 Avail (P)
@ R3 Avail->link, PTR

@ Return 0 or 1

.balign 4
asm_circular_list_insert_left:
	@ 1
	LDR R2, [R0, #CIRCULAR_AVAIL]	@ R2 = Avail
	CBZ R2, overflow

	@ MOVS R3, R2					@ P = Avail

	LDR R3, [R2, #NODE_LINK]		@ R3 = Avail->link;
	STR R3, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = Avail->link;

	@ 2
	STR R1, [R2, #NODE_INFO]		@ P->info = info

	@ R1 and R2 can be reused
	LDR R3, [R0, #CIRCULAR_PTR]
	CBNZ R3, insert_p_at_front

	@ P points to itself
	STR R2, [R2, #NODE_LINK]		@ P->link = P
	STR R2, [R0, #CIRCULAR_PTR]		@ circular_list->ptr = P

	@ done
	MOVS R0, #1
	BX LR

insert_p_at_front:
	LDR R1, [R3, #NODE_LINK]		@ R1 = ptr->link
	STR R1, [R2, #NODE_LINK]		@ P->link = circular_list->ptr->link
	STR R2, [R3, #NODE_LINK]		@ circular_list->ptr->link = P;

done:
	MOVS R0, #1
	BX LR

overflow:
	MOVS R0, #0
	BX LR
