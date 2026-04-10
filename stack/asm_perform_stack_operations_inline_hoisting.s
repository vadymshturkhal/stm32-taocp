.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_stack_operations_inline_hoisting
    .type asm_perform_stack_operations_inline_hoisting, %function


@ Performs max_nodes Push and Pop with integrated Push/Pop

@ Stats with 128 nodes, 128 Push/Pop using Bump Allocator (balloc):
@ cycles_cold = [3299], cycles_warm = 3273, size = 180 bytes

@ Tricks & Insights:
@ Hoisting, Using Registers as Level 0 Cache, Redundant Load Elimination,
@ Deterministic Waterfall Exit
@ Notice that STRD is at least two times slower than two consecutive STR in that case


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
@ R1 loop counter
@ R2 max_nodes
@ R3 Top
@ R4 Avail
@ R5 max_nodes, Next_Avail, Next_Top
@ R6 0 or 1 flag

asm_perform_stack_operations_inline_hoisting:
	PUSH {R4-R6, LR}

	CBZ R0, early_exit

	@ clean values
	UXTH R5, R0					@ R5 = max_nodes
	LSL R0, R5, #3				@ R0 = max_nodes * 8
	ADDS R0, #STACK_SIZE		@ R0 = max_nodes * sizeof(Node) + sizeof(Stack)

	@ allocate memory
	BL asm_balloc
	CBZ R0, early_exit

	@ R0 already contains asm_stack_memory
	MOVS R1, R5					@ Move max_nodes to R1
	BL asm_create_stack			@ Stack* stack = asm_create_stack(asm_stack_memory, max_nodes);
	MOVS R2, R5					@ R2 = max_nodes
	MOVS R6, #0					@ Store false flag to R6
	@ Stack in R0 now

push_loop_init:
	MOVS R1, R2					@ R1 = loop counter
	LDR R3, [R0, #STACK_TOP]	@ R3 = Top
	LDR R4, [R0, #STACK_AVAIL]	@ R4 = Avail

.balign 4
asm_stack_push_inline:
	@ 1
	CBZ R4, handle_overflow_underflow
	LDR R5, [R4, #NODE_LINK]	@ R5 = Avail->link
	STR R1, [R4, #NODE_INFO]	@ Avail->info = iterations
	STR R3, [R4, #NODE_LINK]	@ Avail->link = Top

	MOVS R3, R4					@ Top = Avail
	MOVS R4, R5					@ Avail = Next_Avail

	SUBS R1, R1, #1
	BNE asm_stack_push_inline

store_top_and_avail_after_push:
	STR R3, [R0, #STACK_TOP]
	STR R4, [R0, #STACK_AVAIL]

pop_loop_init:
	MOVS R1, R2					@ R1 = loop counter

	@ R3 and R4 already hold Top and Avail and it's unnecessary to load them
	@ This is an example of Redundant Load Elimination
	@ LDR R3, [R0, #STACK_TOP]	@ R3 = Top
	@ LDR R4, [R0, #STACK_AVAIL]@ R4 = Avail

asm_stack_pop_inline:
	CBZ R3, handle_overflow_underflow
	LDR R5, [R3, #NODE_LINK]	@ R5 = Top->link
	LDR R2, [R3, #NODE_INFO]	@ R2 = top->info;
	STR R4, [R3, #NODE_LINK]	@ Top->link = Avail;

	MOVS R4, R3					@ Avail = Top
	MOVS R3, R5					@ Top = Next_Top

	SUBS R1, R1, #1
	BNE asm_stack_pop_inline

store_flag:
	MOVS R6, #1

@ also a safe exit
handle_overflow_underflow:
	STR R3, [R0, #STACK_TOP]
	STR R4, [R0, #STACK_AVAIL]

done:
	@ Stack pointer is already at R0
	BL asm_balloc_free
	MOVS R0, R6
	POP {R4-R6, PC}

early_exit:
	MOVS R0, #0

exit:
	POP {R4-R6, PC}
