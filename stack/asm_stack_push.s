.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_stack_push


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4

@ Input:
@ R0 Stack* stack
@ R1 uint32_t info

@ Runtime:
@ R0 Stack* stack
@ R1 uint32_t info
@ R2 Avail
@ R3 Top
asm_stack_push:
	@ 1
	LDR R2, [R0, #STACK_AVAIL]	@ R2 = Avail
	CBZ R2, overflow			@ if Avail == NULL: return false

	LDR R3, [R2, #NODE_LINK]	@ R3 = Avail->link
	STR R3, [R0, #STACK_AVAIL]	@ stack->avail = Avail->link

	@ 2
	STR R1, [R2, #NODE_INFO]	@ Avail->info = info

	@ 3
	LDR R3, [R0, #STACK_TOP]	@ R3 = Top
	STR R3, [R2, #NODE_LINK]	@ Avail->link = Top

	@ 4
	STR R2, [R0, #STACK_TOP]	@ stack->top = Avail

done:
	BX LR

overflow:
	MOVS R0, #0
	BX LR
