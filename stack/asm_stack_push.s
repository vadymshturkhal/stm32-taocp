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
@ R2 Node* P = stack->avail
@ R3 Node* P->link
asm_stack_push:
	@ 1
	LDR R2, [R0, #STACK_AVAIL]
	CBZ R2, overflow			@ if (stack->avail == NULL) return false

	@ At the moment R2 = Node* P = stack->avail;
	LDR R3, [R2, #NODE_LINK]	@ R3 = stack->avail->link;
	STR R3, [R0, #STACK_AVAIL]	@ stack->avail = stack->avail->link;

	@ 2
	STR R1, [R2, #NODE_INFO]	@ P->info = info;

	@ 3
	LDR R3, [R0, #STACK_TOP]	@ R2 = stack->top;
	STR R3, [R2, #NODE_LINK]	@ P->link = stack->top;

	@ 4
	STR R2, [R0, #STACK_TOP]	@ stack->top = P;

done:
	MOVS R0, #1
	BX LR

overflow:
	MOVS R0, #0
	BX LR
