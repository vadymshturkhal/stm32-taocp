.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_perform_queue_operations_inline

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

asm_perform_queue_operations_inline:
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

enqueue_loop_init:
	MOVS R6, R4					@ R6 = max_nodes
	LDR R1, [R0, #QUEUE_AVAIL]	@ R1 = P = Avail
	LDR R2, [R0, #QUEUE_REAR]	@ R2 = address held in rear (e.g. &queue->front)
	STR R1, [R2]				@ *queue->rear = Avail (link Rear to Avail List once)

.balign 4
enqueue_loop:
	@ 1
	CBZ R1, overflow			@ if Avail == NULL: return false

	LDR R3, [R1, #NODE_LINK]	@ R3 = P->link
	STR R6, [R1, #NODE_INFO]	@ 2. P->info = info
	@ STR R5, [R1, #NODE_LINK]	@ 3. P->link = NULL only for the last rear
	@ STR R1, [R2]				@ 4. *queue->rear = P
	ADDS R2, R1, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)
	MOVS R1, R3					@ P = P->link

	SUBS R6, R6, #1
	BNE enqueue_loop

enqueue_loop_sync:
	@ last rear link = NULL
	STR R5, [R2]				@ *queue->rear = NULL
	STR R2, [R0, #QUEUE_REAR]	@ queue->rear = &P->link
	STR R1, [R0, #QUEUE_AVAIL]	@ Avail = Avail->link (from stage 1)

dequeue_loop_init:
	@ Don't need to LDR due to R1 synchronization before
	@ LDR R1, [R0, #QUEUE_AVAIL]	@ R1 = Avail
	LDR R2, [R0, #QUEUE_FRONT]	@ R2 = Front = P
	MOVS R6, R4					@ R6 = max_nodes

@ Using R4 since max nodes is no longer needed
.balign 4
dequeue_loop:
	@ 1
	CBZ R2, underflow			@ if Front == NULL: underflow

	LDR R3, [R2, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R4, [R2, #NODE_INFO]	@ 3. R4 = P->info

	@ 4. Avail <= P
	STR R1, [R2, #NODE_LINK]	@ P->link = Avail
	MOVS R1, R2					@ Avail = P
	MOVS R2, R3					@ queue->front = P->link

	SUBS R6, R6, #1
	BNE dequeue_loop

dequeue_loop_sync:
	STR R2, [R0, #QUEUE_FRONT]	@ store queue->front

	@ queue is empty
	CBNZ R2, set_success_flag
	STR R0, [R0, #QUEUE_REAR]	@ queue->rear = &queue->front

set_success_flag:
	MOVS R5, #1

free_queue_memory_store_avail:
	STR R1, [R0, #QUEUE_AVAIL]	@ store Avail
	BL asm_balloc_free

done:
	MOVS R0, R5
	POP {R4-R6, PC}

overflow:
	STR R2, [R0, #QUEUE_REAR]	@ queue->rear = &P->link
	B free_queue_memory_store_avail

underflow:
	STR R2, [R0, #QUEUE_FRONT]	@ store queue->front
	B free_queue_memory_store_avail
