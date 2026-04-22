.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_algorithm_t_sequential
	.type asm_algorithm_t_sequential, %function


@ Info:
@ (QLINK=COUNT) and TOP are interleaving:
@ COUNT[k] is at [R8, k * 8]
@ TOP[k] is at [R8, (k * 8) + 4]


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ QLINK_OFFSET,	0
.equ COUNT_OFFSET,	0
.equ TOP_OFFSET, 	4

@ Input:
@ R0 uint8_t n
@ R1 Pair* input_pairs
@ R2 uint8_t input_pairs_len
@ R3 uint32_t* output

@ Runtime:
@ R0 COUNT and TOP, Avail
@ R1 n, j
@ R2 0, k
@ R3 0
@ R4 n, N
@ R5 input_pairs, REAR pointer
@ R6 input_pairs_len, AVAIL_LIST
@ R7 output
@ R8 COUNT=QLINK and TOP (starts with COUNT + n + 1 shifting)

@ Variables to add:
@ TopologicalNode* P;
@ uint32_t FRONT;
@ uint32_t REAR;
@ Avail

asm_algorithm_t_sequential:
	CMP R0, #0
	BEQ done

	PUSH {R4-R8, LR}

	@ T1
	MOVS R4, R0
	MOVS R5, R1
	MOVS R6, R2
	MOVS R7, R3

allocate_all_memory:
	ADDS R0, #1
	LSL R0, R0, #3		@ asm_balloc((n + 1)*sizeof(uint32_t)) * 2;

	MOVS R1, #NODE_SIZE
	MLA R0, R6, R1, R0	@ R0 + asm_balloc(input_pairs_len * sizeof(TopologicalNode))

	BL asm_balloc
	@ CBZ R0, return_null	@ out of range
	CMP R0, #0
	BEQ return_null

	MOV R8, R0			@ R8 = Master memory pointer for COUNT and TOP

	MOVS R1, R4			@ R1 = n

	EORS R2, R2			@ R2 = 0
	EORS R3, R3			@ R3 = 0

.balign 4
init_COUNT_and_TOP_loop:
	STMIA R0!, {R2, R3}

	SUBS R1, R1, #1		@ k--
	BGE init_COUNT_and_TOP_loop

init_AVAIL_LIST:
	CBZ R6, prepare_to_scan_for_zeros
	@ MOVS R1, R6
	@ BL asm_init_avail_list_t
	@ CBZ R0, handle_relations

@ T2-T3
handle_relations:
	@ R0 = AVAIL_LIST base pointer
	ADD R0, R0, R6, LSL #3	@ R0 (Avail base + input_pairs_len * 8)
	MOVS R1, R5			@ R1 input_pairs
	MOVS R2, R6			@ R2 input_pairs_len
	MOVS R3, R8			@ R3 COUNT and TOP

	BL asm_handle_relations_sequential

@ T4
prepare_to_scan_for_zeros:
	MOVS R0, #0			@ R0 = REAR = 0
	STR R0, [R8]		@ QLINK[0] = 0
	MOVS R1, R4			@ R1 = n
	MOV R5, R8			@ R5 = REAR pointer

.balign 4
scan_for_zeros:
	@ADD R3, R8, R1, LSL #3		@ Address of COUNT[k]
	LDR R2, [R8, R1, LSL #3]				@ R2 = COUNT[k]
	CBNZ R2, continue_to_scan	@ COUNT[k] == 0?

	STR R1, [R5]				@ QLINK[REAR] = k
	ADD R5, R8, R1, LSL #3		@ R5 = REAR pointer = Address of COUNT[k]
	@ MOVS R5, R3				@ Updare REAR pointer

continue_to_scan:
	SUBS R1, R1, #1
	BNE scan_for_zeros

@ R0 Front
@ R3 P
@ R4 N
@ R5 REAR
@ R1, R2, R6 scratch registers

@ T5
prepare_t5:
	LDR R0, [R8]			@ FRONT = QLINK[0];

output_front_of_queue:
	CBZ R0, end_of_process

	STR R0, [R7], #4			@ output[queue_output_counter] = FRONT

	ADD R2, R8, R0, LSL #3		@ Address of TOP[FRONT]
	LDR R3, [R2, #TOP_OFFSET]	@ P = TOP[FRONT]
	CBZ R3, remove_from_queue

@ T6
.balign 4
erase_relations:
	LDR R6, [R3, #NODE_INFO]
	LDR R3, [R3, #NODE_LINK]		@ P = P->next

	LDR R1, [R8, R6, LSL #3]		@ COUNT[P->succ]
	SUBS R1, R1, #1

	@ CMP R1, #0
	ITT EQ
	STREQ R6, [R5]					@ QLINK[REAR] = P->succ;
	ADDEQ R5, R8, R6, LSL #3		@ R5 = REAR = COUNT[P->succ]

	STR R1, [R8, R6, LSL #3]		@ COUNT[P->succ] -= 1

	CMP R3, #0
	BNE erase_relations

@ T7
remove_from_queue:
	ADD R1, R8, R0, LSL #3
	LDR R0, [R1, #QLINK_OFFSET]		@ FRONT = QLINK[FRONT]
	SUBS R4, R4, #1					@ N--
	BNE output_front_of_queue

@ T8
end_of_process:
	CBNZ R4, exception				@ uint8_t topological_status = (N == 0) ? 1 : 0;

done:
	MOV R0, R8
	BL asm_balloc_free

	MOVS R0, #1
	POP {R4-R8, PC}

exception:
	MOV R0, R8
	BL asm_balloc_free

return_null:
	MOVS R0, #0
	POP {R4-R8, PC}
