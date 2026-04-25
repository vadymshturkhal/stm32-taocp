.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_algorithm_t_sequential_stack
	.type asm_algorithm_t_sequential_stack, %function


@ Info:
@ (SLINK=COUNT) and TOP are interleaving:
@ COUNT[k] is at [R8, k * 8]
@ TOP[k] is at [R8, (k * 8) + 4]


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ SLINK_OFFSET,	0
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
@ R5 input_pairs
@ R6 input_pairs_len, AVAIL_LIST
@ R7 output
@ R8 COUNT=SLINK and TOP (starts with COUNT + n + 1 shifting)

asm_algorithm_t_sequential_stack:
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
	CBZ R0, return_null

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
	MOVS R0, #0			@ R0 = STACK_TOP = 0
	MOVS R1, R4			@ R1 = n

.balign 4
scan_for_zeros:
	LDR R2, [R8, R1, LSL #3]	@ R2 = COUNT[k]

	CMP R2, #0
	ITT EQ
	STREQ R0, [R8, R1, LSL #3]	@ SLINK[k] = STACK_TOP
	MOVSEQ R0, R1				@ STACK_TOP = k;

	SUBS R1, R1, #1
	BNE scan_for_zeros

	ADD R5, R8, #TOP_OFFSET		@ R5 = TOP

@ R0 STACK_TOP
@ R3 P
@ R4 N
@ R5 TOP
@ R6 P-> succ
@ R1 tmp
@ R2  free register

@ T5
.balign 4
output_stack_top:
	STR R0, [R7], #4			@ output[queue_output_counter] = STACK_TOP
	SUBS R4, R4, #1				@ N--

	@ ADD R2, R8, R0, LSL #3		@ Address of TOP[STACK_TOP]
	@ LDR R3, [R2, #TOP_OFFSET]	@ P = TOP[STACK_TOP]
	LDR R3, [R5, R0, LSL #3	]	@ P = TOP[STACK_TOP]
	LDR R0, [R8, R0, LSL #3]	@ STACK_TOP = SLINK[STACK_TOP]
	CBZ R3, remove_from_stack

@ T6
erase_relations:
	LDR R6, [R3, #NODE_INFO]		@ P->succ
	LDR R3, [R3, #NODE_LINK]		@ P = P->next

	LDR R1, [R8, R6, LSL #3]		@ COUNT[P->succ]
	SUBS R1, R1, #1
	@ STR R1, [R8, R6, LSL #3]		@ COUNT[P->succ] -= 1

	@ CMP R1, #0
	ITT EQ
	@ STREQ R0, [R8, R6, LSL #3]		@ SLINK[P->succ] = STACK_TOP
	MOVEQ R1, R0					@ Lipski trick (Registers Permutation): Store R0 to R1, as we store R1 anyway
	MOVEQ R0, R6					@ STACK_TOP = P->succ;

	STR R1, [R8, R6, LSL #3]		@ COUNT[P->succ] -= 1

	CMP R3, #0
	BNE erase_relations

remove_from_stack:
	CMP R0, #0
	BNE output_stack_top

@ T7
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
