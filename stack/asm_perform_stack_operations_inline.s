.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_stack_operations_inline


@ Performs max_nodes Push and Pop with integrated Push/Pop


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
@ R4 max_nodes
@ R5 memory pointer
@ R6 max_nodes loop counter

asm_perform_stack_operations_inline:
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
	MOVS R5, R0					@ Save memory pointer to R5
	MOVS R1, R4					@ Move max_nodes to R1
	BL asm_create_stack			@ Stack* stack = asm_create_stack(asm_stack_memory, max_nodes);
	@ Stack in R0 now

push_loop_init:
	MOVS R6, R4

.balign 4
asm_stack_push_inline:
	@ 1
	LDR R2, [R0, #STACK_AVAIL]
	CBZ R2, handle_overflow_underflow			@ if (stack->avail == NULL) return false

	@ At the moment R2 = Node* P = stack->avail;
	LDR R3, [R2, #NODE_LINK]	@ R3 = stack->avail->link;
	STR R3, [R0, #STACK_AVAIL]	@ stack->avail = stack->avail->link;

	@ 2
	STR R6, [R2, #NODE_INFO]	@ P->info = info;

	@ 3
	LDR R3, [R0, #STACK_TOP]	@ R2 = stack->top;
	STR R3, [R2, #NODE_LINK]	@ P->link = stack->top;

	@ 4
	STR R2, [R0, #STACK_TOP]	@ stack->top = P;

	SUBS R6, R6, #1
	BNE asm_stack_push_inline

pop_loop_init:
	MOVS R6, R4

asm_stack_pop_inline:
	@ 1
	LDR R2, [R0, #STACK_TOP]	@ P = stack->top
	CBZ R2, handle_overflow_underflow	@ if (stack->top == NULL): underflow

	@ 2
	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link
	STR R3, [R0, #STACK_TOP]	@ stack->top = P->link

	@ 3
	LDR R1, [R2, #NODE_INFO]	@ info = P->info;

	@ 4
	LDR R3, [R0, #STACK_AVAIL]	@ stack->avail
	STR R3, [R2, #NODE_LINK]	@ P->link = stack->avail;
	STR R2, [R0, #STACK_AVAIL]	@ stack->avail = P;

	SUBS R6, R6, #1
	BNE asm_stack_pop_inline

done:
	MOV R0, R5
	BL asm_balloc_free
	MOVS R0, #1
	POP {R4-R6, PC}

handle_overflow_underflow:
	MOVS R0, R5
	BL asm_balloc_free

early_exit:
	MOVS R0, #0

exit:
	POP {R4-R6, PC}
