.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_dequeue_mve4


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

.equ QUEUE_FRONT, 	0
.equ QUEUE_REAR,	4
.equ QUEUE_AVAIL,	8

@ Input:
@ R0 Queue* queue
@ R1 Number of iterations

@ Runtime:
@ R0 queue
@ R1 Top->info
@ R2 Front
@ R3 P->link, Avail
@ R5 remainder, P->info
@ R6 iterations

@ Return:
@ R0: flag (0 or 1)

asm_dequeue_mve4:
	PUSH {R4-R6, LR}

	MOVS R6, #0					@ set flag to 0
	CBZ R1, early_return

	ANDS R5, R1, #3
	@MOVS R6, R1
	LSRS R6, R1, #2

	LDR R1, [R0, #QUEUE_AVAIL]	@ R1 = Avail
	LDR R2, [R0, #QUEUE_FRONT]	@ R2 = Front = P

	CBZ R5, deque_mve_loop

.balign 4
handle_dequeue_tail_loop:
	CBZ R2, handle_underflow0			@ if Front == NULL: underflow

	LDR R3, [R2, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R5, [R2, #NODE_INFO]	@ 3. R4 = P->info

	@ 4. Avail <= P
	STR R1, [R2, #NODE_LINK]	@ P->link = Avail
	MOVS R1, R2					@ Avail = P
	MOVS R2, R3					@ queue->front = P->link

	@SUBS R6, R6, #1				@ decrement iterations
	SUBS R5, R5, #1				@ decrement tail counter
	BNE handle_dequeue_tail_loop

is_done:
	CBZ R6, store_success_flag

deque_mve_loop:
	@ Front = R2, Avail = R1
	CBZ R2, handle_underflow0	@ if Front == NULL: underflow
	LDR R3, [R2, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R5, [R2, #NODE_INFO]	@ 3. R5 = P->info
	STR R1, [R2, #NODE_LINK]	@ P->link = Avail
	@SUBS R6, R6, #1

	@ Front = R3, Avail = R2
	CBZ R3, handle_underflow1	@ if Front == NULL: underflow
	LDR R4, [R3, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R5, [R3, #NODE_INFO]	@ 3. R5 = P->info
	STR R2, [R3, #NODE_LINK]	@ P->link = Avail
	@SUBS R6, R6, #1

	@ Front = R4, Avail = R3
	CBZ R4, handle_underflow2	@ if Front == NULL: underflow
	LDR R1, [R4, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R5, [R4, #NODE_INFO]	@ 3. R5 = P->info
	STR R3, [R4, #NODE_LINK]	@ P->link = Avail
	@SUBS R6, R6, #1

	@ Front = R1, Avail = R4
	CBZ R1, handle_underflow3	@ if Front == NULL: underflow
	LDR R2, [R1, #NODE_LINK]	@ 2. R3 = P->link;
	LDR R5, [R1, #NODE_INFO]	@ 3. R5 = P->info
	STR R4, [R1, #NODE_LINK]	@ P->link = Avail

	SUBS R6, R6, #1

	BNE deque_mve_loop

store_success_flag:
	MOVS R6, #1

done:
	STR R1, [R0, #QUEUE_AVAIL]	@ store Avail
	STR R2, [R0, #QUEUE_FRONT]	@ store queue->front

early_return:
	MOVS R0, R6
	POP {R4-R6, PC}

handle_underflow0:
	STR R1, [R0, #QUEUE_AVAIL]	@ store Avail
	STR R2, [R0, #QUEUE_FRONT]	@ store queue->front
	B underflow

handle_underflow1:
	STR R2, [R0, #QUEUE_AVAIL]	@ store Avail
	STR R3, [R0, #QUEUE_FRONT]	@ store queue->front
	B underflow

handle_underflow2:
	STR R3, [R0, #QUEUE_AVAIL]	@ store Avail
	STR R4, [R0, #QUEUE_FRONT]	@ store queue->front
	B underflow

handle_underflow3:
	STR R4, [R0, #QUEUE_AVAIL]	@ store Avail
	STR R1, [R0, #QUEUE_FRONT]	@ store queue->front

underflow:
	MOVS R0, #0
	POP {R4-R6, PC}
