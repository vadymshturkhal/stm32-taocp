.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_queue_operations


@ Performs max_nodes Push and Pop

@ Memory offsets
.equ NODE_SIZE,		8	@ sizeof(Node) = 8 bytes
.equ QUEUE_SIZE,	12	@ sizeof(Queue) = 12 bytes

@ Input:
@ R0 uint16_t max_nodes

@ Runtime:
@ R0 max_nodes, max_nodes * sizeof(Node) + sizeof(Queue)
@ R4 max_nodes
@ R5 memory pointer
@ R6 max_nodes loop counter
@ R7 Queue pointer

@ Return 0 if false or 1 if success run

asm_perform_queue_operations:
	PUSH {R4-R8, LR}			@ add R8 for 8-byte stack alignment

	CBZ R0, early_exit

	@ clean values
	UXTH R4, R0

	LSL R0, R4, #3				@ R0 = max_nodes * 8
	ADDS R0, #QUEUE_SIZE		@ R0 = max_nodes * sizeof(Node) + sizeof(Queue)

	@ allocate memory
	BL asm_balloc
	CBZ R0, early_exit

	@ R0 already contains asm_queue_memory
	MOVS R5, R0					@ Save memory pointer to R5
	MOVS R1, R4					@ Move max_nodes to R1
	BL asm_create_queue			@ Queue* queue = asm_create_queue(asm_queue_memory, max_nodes);
	MOVS R7, R0

enqueue_loop_init:
	MOVS R6, R4

.balign 4
enqueue_loop:
	MOVS R0, R7					@ Move Queue pointer to R0
	MOVS R1, R6					@ Move info to R1
	BL asm_enqueue

	CBZ R0, handle_overflow_underflow

	SUBS R6, R6, #1
	BNE enqueue_loop

dequeue_loop_init:
	MOVS R6, R4

.balign 4
dequeue_loop:
	MOVS R0, R7					@ Move Queue pointer to R0
	BL asm_dequeue

	@ now R0 contains flag and R1 contains info
	CBZ R0, handle_overflow_underflow	@ check status

	SUBS R6, R6, #1
	BNE dequeue_loop

done:
	MOVS R0, R5
	BL asm_balloc_free
	MOVS R0, #1
	POP {R4-R8, PC}

handle_overflow_underflow:
	MOVS R0, R5
	BL asm_balloc_free

early_exit:
	MOVS R0, #0

exit:
	POP {R4-R8, PC}
