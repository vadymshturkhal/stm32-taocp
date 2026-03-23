.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_stack_operations


@ Performs max_nodes Push and Pop

@ Memory offsets
.equ NODE_SIZE,		8	@ sizeof(Node) = 8 bytes
.equ STACK_SIZE,	8	@ sizeof(Stack) = 8 bytes

@ Input:
@ R0 uint16_t max_nodes

@ Runtime:
@ R0 max_nodes, max_nodes * sizeof(Node) + sizeof(Stack)
@ R4 max_nodes
@ R5 memory pointer
@ R6 max_nodes loop counter
@ R7 Stack pointer

asm_perform_stack_operations:
	PUSH {R4-R7, LR}			@ add R7 for stack alignment
	SUBS SP, SP, #4				@ for bool pop_is_success

	CBZ R0, early_exit

	@ clean values
	UXTH R4, R0
	MOVS R6, #1
	STRB R6, [SP]				@ bool pop_is_success = true

	LSL R0, R4, #3				@ R0 = max_nodes * 8
	ADDS R0, #STACK_SIZE		@ R0 = max_nodes * sizeof(Node) + sizeof(Stack)

	@ allocate memory
	BL asm_balloc
	CBZ R0, early_exit

	@ R0 already contains asm_stack_memory
	MOVS R5, R0					@ Save memory pointer to R5
	MOVS R1, R4					@ Move max_nodes to R1
	BL asm_create_stack			@ Stack* stack = asm_create_stack(asm_stack_memory, max_nodes);
	MOVS R7, R0

push_loop_init:
	MOVS R6, R4

.balign 4
push_loop:
	MOVS R0, R7					@ Move Stack pointer to R0
	MOVS R1, R6					@ Move info to R1
	BL asm_stack_push

	CBZ R0, handle_overflow_underflow

	SUBS R6, R6, #1
	BNE push_loop

pop_loop_init:
	MOVS R6, R4

pop_loop:
	MOVS R0, R7					@ Move Stack pointer to R0
	MOV R1, SP					@ Move &pop_is_success to R1
	BL asm_stack_pop

	@ now R0 contains info
	LDRB R2, [SP]
	CBZ R2, handle_overflow_underflow	@ check pop_is_success flag

	SUBS R6, R6, #1
	BNE pop_loop

done:
	MOV R0, R5
	BL asm_balloc_free
	MOVS R0, #1
	ADDS SP, SP, #4
	POP {R4-R7, PC}

handle_overflow_underflow:
	MOVS R0, R5
	BL asm_balloc_free

early_exit:
	MOVS R0, #0

exit:
	ADDS SP, SP, #4
	POP {R4-R7, PC}
