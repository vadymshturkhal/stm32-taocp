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
@ R0 bool* pop_is_success
@ R1 Stack* stack

@ Runtime:
@ R0 bool* pop_is_success, info
@ R1 Stack* stack
@ R2 Node* P = stack->top
@ R3 Node* P->link then stack->avail

@ Return:
@ Top (node info) or (0 and update pop_is_success flag)
asm_stack_pop:
	@ 1
	LDR R2, [R1, #STACK_TOP]	@ R2 = Top
	CBZ R2, underflow			@ if Top == NULL: underflow

	@ 2
	LDR R3, [R2, #NODE_LINK]	@ R3 = Next Top
	STR R3, [R1, #STACK_TOP]	@ stack->top = Next Top

	@ 3
	LDR R0, [R2, #NODE_INFO]	@ R0 = Top->info;

	@ 4
	LDR R3, [R1, #STACK_AVAIL]	@ R3 = Avail
	STR R3, [R2, #NODE_LINK]	@ Top->link = Avail;
	STR R2, [R1, #STACK_AVAIL]	@ Avail = Top;

done:
	@ info is already at R0
	BX LR						@ return info;

underflow:
	MOVS R3, #0
	STRB R3, [R0]				@ *pop_is_success = false

	MOVS R0, #0
	BX LR						@ Return 0
