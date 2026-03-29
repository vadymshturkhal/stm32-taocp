.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_stack_pop

@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4

@ Input:
@ R0 Stack* stack

@ Runtime:
@ R0 stack
@ R1 Top->info
@ R2 Top
@ R3 Node* P->link then stack->avail

@ Return:
@ Top flag (0 or 1) and info
asm_stack_pop:
	@ 1
	LDR R2, [R0, #STACK_TOP]	@ R2 = Top
	CBZ R2, underflow			@ if Top == NULL: underflow

	@ 2
	LDR R3, [R2, #NODE_LINK]	@ R3 = Next Top
	STR R3, [R0, #STACK_TOP]	@ stack->top = Next Top

	@ 3
	LDR R1, [R2, #NODE_INFO]	@ R1 = Top->info;

	@ 4
	LDR R3, [R0, #STACK_AVAIL]	@ R3 = Avail
	STR R3, [R2, #NODE_LINK]	@ Top->link = Avail;
	STR R2, [R0, #STACK_AVAIL]	@ Avail = Top;

done:
	@ info is already at R1
	MOVS R0, #1
	BX LR						@ return flag and info;

underflow:
	MOVS R0, #0
	BX LR						@ Return 0
