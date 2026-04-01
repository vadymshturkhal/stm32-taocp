.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_dequeue


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ QUEUE_FRONT, 	0
.equ QUEUE_REAR,	4
.equ QUEUE_AVAIL,	8

@ Input:
@ R0 Queue* queue

@ Runtime:
@ R0 queue
@ R1 Top->info
@ R2 Front
@ R3 P->link, Avail

@ Return:
@ R0: flag (0 or 1)
@ R1: info

asm_dequeue:
	@ 1
	LDR R2, [R0, #QUEUE_FRONT]	@ R2 = Front = P
	CBZ R2, underflow			@ if Top == NULL: underflow

	@ 2
	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link;
	@ STR R3, [R0, #QUEUE_FRONT]	@ queue->front = P->link (we'll do that in stage 5)

	@ 4. Avail <= P
	LDR R1, [R0, #QUEUE_AVAIL]	@ R1 = Avail
	STR R1, [R2, #NODE_LINK]	@ P->link = Avail
	STR R2, [R0, #QUEUE_AVAIL]	@ Avail = P

	@ 3
	LDR R1, [R2, #NODE_INFO]	@ R1 = P->info

	@ 5. queue->front == NULL?
	STR R3, [R0, #QUEUE_FRONT]	@ queue->front = P->link (from stage 2)
	CBZ R3, front_is_null

done:
	@ info is already at R1
	MOVS R0, #1
	BX LR						@ return 1 and info

underflow:
	MOVS R0, #0
	BX LR						@ return 0

front_is_null:
	@ point Rear to the memory address of Front
	STR R0, [R0, #QUEUE_REAR]	@ queue->rear = &queue->front
	MOVS R0, #1
	BX LR						@ return 1 and info
