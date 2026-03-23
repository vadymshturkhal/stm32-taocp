.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_create_stack


@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8
.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4
.equ STACK_SIZE,	8


@ Input
@ R0 void* memory
@ R1 uint32_t size

@ Runtime
@ R0 stack
@ R1 size
@ R2 NULL or 0, tmp
@ R3 avail

@ Return Stack
asm_create_stack:
	MOVS R2, #0
	STR R2, [R0, #STACK_TOP]	@ stack->top = NULL

init_storage_pool:
	ADDS R3, R0, #STACK_SIZE	@ Node* avail = (Node*)(stack + 1), since Stack is 8 bytes
	STR R1, [R3, #NODE_INFO]	@ avail->info = size
	STR R2, [R3, #NODE_LINK]	@ avail->link = NULL

	SUBS R1, R1, #1				@ size--
	CBZ R1, done				@ if stack is len 1

linking_loop:
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1
	STR R1, [R2, #NODE_INFO]	@ tmp->info = size
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail

	MOVS R3, R2					@ avail = tmp
	SUBS R1, R1, #1				@ size--
	BNE linking_loop

@ now R3 is Node* avail
done:
	STR R3, [R0, #STACK_AVAIL]	@ stack->avail = avail;
	BX LR
