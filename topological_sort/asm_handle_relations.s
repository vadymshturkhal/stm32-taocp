.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_handle_relations
	.type asm_handle_relations, %function


@ Info: does not return any error


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ TOP_OFFSET, 	4

@ Input:
@ R0 Avail
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
asm_handle_relations:
	PUSH {R4-R7, LR}
	CBZ R2, done

	@ Set R1 to point to the END of input_pairs
	ADDS R1, R1, R2, LSL #3		@ R2 = R2 + (input_pairs_len * pair_size)

@ T2 (Next relation)
.balign 4
handle_relations_loop:
	@ LDRD R4, R5, [R1, #-8]!
	LDR R5, [R1, #-4]!		@ Load successor (k) from 4 bytes ahead of the newly updated R1
	LDR R4, [R1, #-4]!		@ R1 -= 8 (Writeback!); Load predecessor (j) from the new R1

	@ T3
	LDR R7, [R3, R5, LSL #3]	@ R7 = COUNT[k]
	ADDS R6, R3, R4, LSL #3		@ R6 = TOP[j] memory address
	ADDS R7, R7, #1				@ COUNT[k]++
	STR R7, [R3, R5, LSL #3]	@ Store COUNT[k]

	MOVS R7, R0					@ P = Avail
	CBZ R7, overflow			@ P == NULL?

	LDR R0, [R0, #NODE_LINK]	@ Avail = Avail->link
	STR R5, [R7, #NODE_INFO]	@ P->info = k

	LDR R5, [R6, #TOP_OFFSET]	@ R5 = TOP[j]
	STR R5, [R7, #NODE_LINK]	@ P->next = TOP[j]

	STR R7, [R6, #TOP_OFFSET]	@ TOP[j] = P

	SUBS R2, R2, #1
	BNE handle_relations_loop

done:
	MOVS R0, #1
	POP {R4-R7, PC}

overflow:
	MOVS R0, #0
	POP {R4-R7, PC}
