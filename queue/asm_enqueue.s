.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_enqueue
    .type asm_enqueue, %function


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ QUEUE_FRONT, 	0
.equ QUEUE_REAR,	4
.equ QUEUE_AVAIL,	8

@ Input:
@ R0 Queue* queue
@ R1 uint32_t info

@ Runtime:
@ R0 Queue* queue
@ R1 uint32_t info
@ R2 Avail, P
@ R3 = P->link

@ Return: 1 if success else 0

asm_enqueue:
	@ 1
	LDR R2, [R0, #QUEUE_AVAIL]	@ R2 = P = Avail
	CBZ R2, overflow			@ if Avail == NULL: return false

	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link
	@ STR R3, [R0, #QUEUE_AVAIL]	@ Avail = Avail->link (we'll do that in stage 4)

	@ 2. P->info = info
	STR R1, [R2, #NODE_INFO]

	@ 3. P->link = NULL
	MOVS R1, #0
	STR R1, [R2, #NODE_LINK]

	@ 4 (queue->rear->link = P)
	@ if empty: writes P into queue->front, else: into the old rear's link pointer
	LDR R1, [R0, #QUEUE_REAR]	@ R1 = address held in rear (e.g. &queue->front)
	STR R3, [R0, #QUEUE_AVAIL]	@ Avail = Avail->link (from stage 1)
	STR R2, [R1]				@ *queue->rear = P

	@ 5 (queue->rear = P)
	ADDS R1, R2, #NODE_LINK		@ R1 = Address of P->link (&P->link)
	STR R1, [R0, #QUEUE_REAR]	@ queue->rear = &P->link

done:
	MOVS R0, #1
	BX LR

overflow:
	MOVS R0, #0
	BX LR
