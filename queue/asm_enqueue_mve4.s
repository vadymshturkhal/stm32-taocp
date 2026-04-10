.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_enqueue_mve4
	.type asm_enqueue_mve4, %function

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
@ R1 Number of iterations, Avail
@ R2 Rear
@ R3 P->link
@ R5 tail, 0
@ R6 mve iterations counter, flag (0 or 1)

@ Return: 1 if success else 0

asm_enqueue_mve4:
	PUSH {R4-R6, LR}

	MOVS R6, #0					@ set flag to 0
	CBZ R1, early_return

	@MOVS R6, R1
	LSRS R6, R1, #2
	ANDS R5, R1, #3

	LDR R1, [R0, #QUEUE_REAR]	@ R1 = address held in rear (e.g. &queue->front)
	LDR R2, [R0, #QUEUE_AVAIL]	@ R2 = P = Avail
	STR R2, [R1]

	CBZ R5, enqueue_mve_loop

.balign 4
handle_enqueue_tail_loop:
	CBZ R2, overflow			@ if Avail == NULL: return false

	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link
	STR R6, [R2, #NODE_INFO]	@ 2. P->info = info
	STR R2, [R1]				@ 4. *queue->rear = P

	ADDS R1, R2, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)
	MOVS R2, R3					@ P = P->link

	SUBS R5, R5, #1
	BNE handle_enqueue_tail_loop

is_done:
	CBZ R6, store_success_flag
	STR R2, [R1]

@ R5 is 0 at the moment
@ set_null_pointer:
@	MOVS R5, #0

.balign 4
enqueue_mve_loop:
	@ R1 = Rear, R2 = Avail
	CBZ R2, overflow	@ if Avail == NULL: return false
	LDR R3, [R2, #NODE_LINK]	@ R3 = P->link
	STR R6, [R2, #NODE_INFO]	@ 2. P->info = info (iterations)
	@STR R2, [R1]				@ 4. *queue->rear = P
	@ ADDS R2, R2, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)

	@ R2 = Rear, R3 = Avail
	CBZ R3, handle_overflow1	@ if Avail == NULL: return false
	LDR R4, [R3, #NODE_LINK]	@ R3 = P->link
	STR R6, [R3, #NODE_INFO]	@ 2. P->info = info (iterations)
	@STR R3, [R2]				@ 4. *queue->rear = P
	@ ADDS R3, R3, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)

	@ R3 = Rear, R4 = Avail
	CBZ R4, handle_overflow2	@ if Avail == NULL: return false
	LDR R1, [R4, #NODE_LINK]	@ R3 = P->link
	STR R6, [R4, #NODE_INFO]	@ 2. P->info = info (iterations)
	@STR R4, [R3]				@ 4. *queue->rear = P
	@ ADDS R4, R4, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)

	@ R4 = Rear, R1 = Avail
	CBZ R1, handle_overflow3	@ if Avail == NULL: return false
	LDR R2, [R1, #NODE_LINK]	@ R3 = P->link
	STR R6, [R1, #NODE_INFO]	@ 2. P->info = info (iterations)
	@ STR R1, [R4]				@ 4. *queue->rear = P
	@ ADDS R1, R1, #NODE_LINK		@ 5. R2 = Address of P->link (&P->link)

	SUBS R6, R6, #1
	BNE enqueue_mve_loop

store_success_flag:
	MOVS R6, #1

done:
	STR R5, [R1, #NODE_LINK]	@ queue->rear = NULL
	ADDS R1, R1, #NODE_LINK		@ Torvalds Tax
	STR R1, [R0, #QUEUE_REAR]	@ queue->rear = &P->link
	STR R2, [R0, #QUEUE_AVAIL]	@ Avail = Avail->link (from stage 1)

early_return:
	MOVS R0, R6
	POP {R4-R6, PC}

handle_overflow1:
	MOVS R1, R2					@ R1 = Rear
	MOVS R2, R3					@ R2 = Avail
	B overflow

handle_overflow2:
	MOVS R1, R3					@ R1 = Rear
	MOVS R2, R4					@ R2 = Avail
	B overflow

handle_overflow3:
	MOVS R2, R1					@ R2 = Avail
	MOVS R1, R4					@ R1 = Rear

overflow:
	MOVS R6, #0					@ set false flag
	B done
