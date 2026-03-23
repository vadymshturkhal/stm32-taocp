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
@ R1 bool* pop_is_success

@ Runtime:
@ R0 Stack* stack
@ R1 bool* pop_is_success then info
@ R2 Node* P = stack->top
@ R3 Node* P->link then stack->avail

@ Return:
@ Top (node info) or (0 and update pop_is_success flag)
asm_stack_pop:
	@ 1
	LDR R2, [R0, #STACK_TOP]	@ P = stack->top
	CBZ R2, underflow			@ if (stack->top == NULL): underflow

	@ 2
	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link
	STR R3, [R0, #STACK_TOP]	@ stack->top = P->link

	@ 3
	LDR R1, [R2, #NODE_INFO]	@ info = P->info;

	@ 4
	LDR R3, [R0, #STACK_AVAIL]	@ stack->avail
	STR R3, [R2, #NODE_LINK]	@ P->link = stack->avail;
	STR R2, [R0, #STACK_AVAIL]	@ stack->avail = P;

done:
	MOVS R0, R1
	BX LR						@ @ return info;

underflow:
	MOVS R3, #0
	STRB R3, [R1]				@ *pop_is_success = false

	MOVS R0, #0
	BX LR						@ Return 0
