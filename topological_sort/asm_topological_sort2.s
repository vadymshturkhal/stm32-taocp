.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_algorithm_t2
	.type asm_algorithm_t2, %function


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4

@ Input:
@ R0 uint8_t n
@ R1 Pair* input_pairs
@ R2 uint8_t input_pairs_len
@ R3 uint32_t* output

@ Runtime:
@ R4 n, N
@ R5 input_pairs
@ R6 input_pairs_len
@ R7 output
@ R8 COUNT
@ R9 QLINK
@ R10 TOP

@ Variables to add:
@ TopologicalNode* P;
@ uint32_t FRONT;
@ uint32_t REAR;
@ Avail List
@ Avail

@ uint32_t* COUNT = asm_balloc((n + 1)*sizeof(uint32_t));
@ TopologicalNode** TOP = asm_balloc((n + 1) * sizeof(TopologicalNode*));
@ AVAIL_LIST = asm_balloc(input_pairs_len * sizeof(TopologicalNode));
asm_algorithm_t2:
	PUSH {R4-R10, LR}

	@ T1
	MOVS R4, R0
	MOVS R5, R1
	MOVS R6, R2
	MOVS R7, R3

init_COUNT:
	@ asm_balloc((n + 1)*sizeof(uint32_t));
	ADDS R0, #1
	LSL R0, R0, #2
	BL asm_balloc
	CBZ R0, return_null
	MOVS R8, R0			@ R8 = COUNT

	MOVS R1, R4
	ADDS R1, #1			@ R1 = n + 1 FIXME (Maybe we should not add)
	MOVS R2, #0

zero_COUNT_loop:
	SUBS R1, #1
	STR R2, [R8, R1, LSL #2]
	BNE zero_COUNT_loop

create_QLINK:
	MOVS R9, R8			@ R9 = COUNT

init_TOP:
	@TopologicalNode** TOP = asm_balloc((n + 1) * sizeof(TopologicalNode*));
	@ if (TOP == NULL) goto exceptions;
	MOVS R0, R4
	ADDS R0, #1
	LSL R0, R0, #2
	BL asm_balloc
	CBZ R0, return_null

	MOVS R10, R0		@ R10 = TOP
	MOVS R1, R4
	ADDS R1, #1			@ R1 = n + 1 FIXME (Maybe we should not add)
	MOVS R2, #0

zero_TOP_loop:
	SUBS R1, #1
	STR R2, [R10, R1, LSL #2]
	BNE zero_COUNT_loop

@ 	uint32_t N = n;

init_avail_list:
	@ CBZ R6,




return_null:
	MOVS R0, #0
	POP {R4-R10, PC}

