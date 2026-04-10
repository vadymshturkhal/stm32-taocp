.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_queue_operations_mve4
	.type asm_perform_queue_operations_mve4, %function

@ Performs max_nodes Enqueue and Dequeue
@ Stats with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc):
@ cycles_cold = [3430-3452], cycles_warm = [3405-3414], size = 194 bytes

@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8	@ sizeof(Node) = 8 bytes

.equ QUEUE_FRONT, 	0
.equ QUEUE_REAR,	4
.equ QUEUE_AVAIL,	8
.equ QUEUE_SIZE,	12	@ sizeof(Queue) = 12 bytes

@ Input:
@ R0 uint16_t max_nodes

@ Runtime:
@ R0 max_nodes, max_nodes * sizeof(Node) + sizeof(Queue)
@ R1 Avail
@ R2 Rear, Front, P
@ R3 P->link
@ R4 max_nodes, P->info
@ R5 = 0 for NULL linkage and false flag
@ R6 max_nodes loop counter

@ Return 0 if false or 1 if success run

asm_perform_queue_operations_mve4:
	PUSH {R4-R6, LR}

	MOVS R5, #0
	CBZ R0, done

	@ clean values
	UXTH R4, R0

	LSL R0, R4, #3				@ R0 = max_nodes * 8
	ADDS R0, #QUEUE_SIZE		@ R0 = max_nodes * sizeof(Node) + sizeof(Queue)

	@ allocate memory
	BL asm_balloc
	CBZ R0, done

	@ R0 already contains asm_queue_memory
	MOVS R1, R4					@ Move max_nodes to R1
	BL asm_create_queue			@ Queue* queue = asm_create_queue(asm_queue_memory, max_nodes);

	@ R0 = Queue* queue and has the same starting address as the memory address from asm_balloc
	MOVS R6, R0

.balign 4
enqueue_mve4:
	@ R0 already == Queue
	MOVS R1, R4					@ R1 = max_nodes
	BL asm_enqueue_mve4
	CBZ R0, handle_overflow_underflow

.balign 4
dequeue_mve4:
	MOVS R0, R6					@ Queue to R0
	MOVS R1, R4					@ R1 = max_nodes
	BL asm_dequeue_mve4
	CBZ R0, handle_overflow_underflow
	MOVS R0, R6					@ Queue to R0

set_success_flag:
	MOVS R5, #1

free_queue_memory:
	BL asm_balloc_free

done:
	MOVS R0, R5
	POP {R4-R6, PC}

handle_overflow_underflow:
	MOVS R0, R6					@ Stack to R0
	B free_queue_memory
