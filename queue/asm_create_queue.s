.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_create_queue
	.type asm_create_queue, %function

@ don't init node->info
@ takes 948 cycles for 128 nodes

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ QUEUE_FRONT, 	0
.equ QUEUE_REAR,	4
.equ QUEUE_AVAIL,	8
.equ QUEUE_SIZE,	12


@ Input
@ R0 void* memory
@ R1 uint32_t size

@ Runtime
@ R0 queue
@ R1 size
@ R2 NULL or 0, tmp
@ R3 avail

@ Return Queue or 0
asm_create_queue:
	MOVS R2, #0
	STR R2, [R0, #QUEUE_FRONT]	@ queue->front = NULL

	@ ADDS R3, R0, #QUEUE_FRONT is redundant due to #QUEUE_FRONT = 0
	STR R0, [R0, #QUEUE_REAR]	@ queue->rear = &queue->front
	CBZ R1, early_exit			@ queue size == 0?

.balign 4
init_storage_pool:
	ADDS R3, R0, #QUEUE_SIZE	@ Node* avail = (Node*)(queue + 1), since Queue is 12 bytes
	@ STR R1, [R3, #NODE_INFO]	@ avail->info = size
	STR R2, [R3, #NODE_LINK]	@ avail->link = NULL

	SUBS R1, R1, #1				@ size--
	CBZ R1, done				@ if queue is len 1

.balign 4
linking_loop:
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1
	@ STR R1, [R2, #NODE_INFO]	@ tmp->info = size
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail

	MOVS R3, R2					@ avail = tmp
	SUBS R1, R1, #1				@ size--
	BNE linking_loop

@ now R3 is Node* avail
done:
	STR R3, [R0, #QUEUE_AVAIL]	@ queue->avail = avail;
	BX LR

early_exit:
	STR R2, [R0, #QUEUE_AVAIL]	@ queue->avail = NULL;
	BX LR
