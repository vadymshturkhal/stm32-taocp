.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_stack_operations_inline_hoisting


@ Performs max_nodes Push and Pop with integrated Push/Pop with Hoisting


@ Memory offsets
.equ NODE_SIZE,		8	@ sizeof(Node) = 8 bytes
.equ STACK_SIZE,	8	@ sizeof(Stack) = 8 bytes

.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4

@ Input:
@ R0 uint16_t max_nodes

@ Runtime:
@ R0 max_nodes, max_nodes * sizeof(Node) + sizeof(Stack), Stack pointer
@ R1 stack->top
@ R2 stack->avail
@ R3 P
@ R4 max_nodes
@ R5 memory pointer
@ R6 max_nodes loop counter

asm_perform_stack_operations_inline_hoisting:
	PUSH {R4-R6, LR}			@ add R7 for stack alignment

	CBZ R0, early_exit

	@ clean values
	UXTH R4, R0
	MOVS R6, #1

	LSL R0, R4, #3				@ R0 = max_nodes * 8
	ADDS R0, #STACK_SIZE		@ R0 = max_nodes * sizeof(Node) + sizeof(Stack)

	@ allocate memory
	BL asm_balloc
	CBZ R0, early_exit

	@ R0 already contains asm_stack_memory
	MOVS R1, R4					@ Move max_nodes to R1
	BL asm_create_stack			@ Stack* stack = asm_create_stack(asm_stack_memory, max_nodes);
	@ Stack in R0 now

push_loop_init:
	MOVS R6, R4

	@ Hoisting
	LDR R1, [R0, #STACK_TOP]	@ R1 = stack->top;
	LDR R2, [R0, #STACK_AVAIL]	@ R2 = stack->avail

.balign 4
asm_stack_push_inline:
	CBZ R2, handle_overflow_underflow		@ if (stack->avail == NULL) return false

	@ R2 is P at the moment
	LDR R3, [R2, #NODE_LINK]	@ R3 = next avail = P->link;
	STR R6, [R2, #NODE_INFO]	@ P->info = info;
	STR R1, [R2, #NODE_LINK]	@ P->link = stack->top;

	MOVS R1, R2					@ R1 = stack->top = P
	MOVS R2, R3					@ stack->avail = next avail

	SUBS R6, R6, #1
	BNE asm_stack_push_inline

push_inline_sync:
	STR R1, [R0, #STACK_TOP]
	STR R2, [R0, #STACK_AVAIL]	@ stack->avail = stack->avail->link

pop_loop_init:
	MOVS R6, R4

	@ Hoisting
	LDR R1, [R0, #STACK_TOP]	@ R1 = stack->top;
	LDR R2, [R0, #STACK_AVAIL]	@ R2 = stack->avail

.balign 4
asm_stack_pop_inline:
	@ 1
	CBZ R1, handle_overflow_underflow	@ if (stack->top == NULL): underflow

	LDR R3, [R1, #NODE_LINK]	@ R3 = P->link
	LDR R5, [R1, #NODE_INFO]	@ info = P->info;
	STR R2, [R1, #NODE_LINK]	@ P->link = stack->avail;

	MOVS R2, R1					@ R2 = stack->avail = P;
	MOVS R1, R3					@ R1 = stack->top = P->link

	SUBS R6, R6, #1
	BNE asm_stack_pop_inline

pop_inline_sync:
	STR R1, [R0, #STACK_TOP]	@ stack->top = P->link
	STR R2, [R0, #STACK_AVAIL]	@ R2 = stack->avail

done:
	BL asm_balloc_free
	MOVS R0, #1
	POP {R4-R6, PC}

handle_overflow_underflow:
	STR R1, [R0, #STACK_TOP]
	STR R2, [R0, #STACK_AVAIL]	@ stack->avail = stack->avail->link
	BL asm_balloc_free

early_exit:
	MOVS R0, #0

exit:
	POP {R4-R6, PC}
