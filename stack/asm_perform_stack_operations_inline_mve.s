.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_stack_operations_inline_mve


@ Performs max_nodes Push and Pop with integrated Push/Pop with Hoisting and MVE 4

@ Stats:
@ with 128 nodes, 128 Push and 128 Pop using balloc (custom malloc)
@ cycles_cold = [2615-2626], cycles_warm = [2554-2555], size = 390 bytes


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

asm_perform_stack_operations_inline_mve:
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
	MOVS R6, R0					@ save Stack

.balign 4
stack_push_mve:
    MOVS R1, R4					@ iterations to R1
  	BL asm_stack_push_mve
  	CBZ R0, handle_overflow_underflow

.balign 4
stack_pop_mve:
	MOVS R0, R6					@ Stack to R0
	MOVS R1, R4					@ iterations to R1
	BL asm_stack_pop_mve
	CBZ R0, handle_overflow_underflow

done:
	MOVS R0, R6					@ Stack to R0
	BL asm_balloc_free
	MOVS R0, #1					@ return true
	POP {R4-R6, PC}

handle_overflow_underflow:
	MOVS R0, R6					@ Stack to R0
	BL asm_balloc_free

early_exit:
	MOVS R0, #0					@ return false

exit:
	POP {R4-R6, PC}
