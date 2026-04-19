.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_handle_relations_sequential
	.type asm_handle_relations_sequential, %function

@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ TOP_OFFSET, 	4

@ Input:
@ R0 Avail List END pointer (Avail base + input_pairs_len * 8)
@ R1 input_pairs
@ R2 input_pairs_len
@ R3 COUNT and TOP

@ Runtime
@ R0 Avail
@ R1 input_pairs
@ R2 input_pairs_len
@ R3 COUNT and TOP
@ R4 j
@ R5 k
@ R6 COUNT[k]
@ R7 P

@ Return 0 or 1
asm_handle_relations_sequential:
	PUSH {R4-R7, LR}
	CBZ R2, done

	@ Set R1 to point to the END of input_pairs
	ADDS R1, R1, R2, LSL #3		@ R1 = R1 + (input_pairs_len * pair_size)

@ T2 (Next relation)
.balign 4
handle_relations_loop:
	@ LDRD R4, R5, [R1, #-8]!
	@ LDMIA R1!, {R4, R5}

	LDR R5, [R1, #-4]!
	LDR R4, [R1, #-4]!

	@ T3
	@ Increment COUNT[k]
	LDR R7, [R3, R5, LSL #3]	@ R7 = COUNT[k]
	STR R5, [R0, #-8]!			@ Avail -= 8; Avail->succ = k
	ADDS R7, R7, #1				@ COUNT[k]++
	STR R7, [R3, R5, LSL #3]	@ Store COUNT[k]

	@ Load TOP[j]
	ADD R7, R3, R4, LSL #3		@ R7 = TOP[j] memory address
	LDR R6, [R7, #TOP_OFFSET]	@ R8 = TOP[j]

	@ STRD R5, R6, [R0, #-8]!
	@ STMDB R0!, {R5, R6}			@ Avail -= 8; Avail->succ = k; Avail->next = TOP[j]

	@ Split STMDB into two STR instructions:
	@ STR R5, [R0, #-8]!			@ Avail -= 8; Avail->succ = k
	STR R6, [R0, #4]			@ Avail->next = TOP[j]

	STR R0, [R7, #TOP_OFFSET]	@ TOP[j] = Avail (our newly allocated node)

	SUBS R2, R2, #1
	BNE handle_relations_loop

done:
	MOVS R0, #1
	POP {R4-R7, PC}
