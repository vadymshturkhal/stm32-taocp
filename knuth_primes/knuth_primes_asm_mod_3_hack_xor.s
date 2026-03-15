.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_knuth_primes_mod_3_hack_xor


@ using mod 3 hack xor filter (adding 2 or 4 to candidate)


@ Input:
@ R0 primes_array
@ R1 primes_to_find

@ Runtime:
@ R1 end pointer
@ R2 current_prime pointer
@ R3 candidate
@ R4 traversal_primes pointer
@ R5 divisor
@ R6 Q
@ R7 temp (divisor * Q)
@ R8 xor step toggle (2 or 4)


asm_knuth_primes_mod_3_hack_xor:
	@ P1
	PUSH {R4-R8, LR}

	@ store 2 and 3 to primes_array
	MOVS R3, #2
	STR R3, [R0, #0]
    MOVS R3, #3
    STR R3, [R0, #4]

	ADDS R2, R0, #8			@ R2 points to primes_array[2]
	ADDS R1, R0, R1, LSL #2	@ end_pointer = primes_array + (primes_to_find * 4)

	MOVS R8, #2				@ init step toggle with 2
	MOVS R3, #5				@ start candidate = 5
	B candidate_is_prime	@ add 5 to primes array

@ P4
advance_candidate:
	ADDS R3, R3, R8			@ add step 2 or 4 to candidate
	EOR R8, R8, #6			@ hack: flips R8 between 2 and 4

@ P5
init_primes_counter:
	ADDS R4, R0, #8         @ skip 2 and 3 from primes_array[0] and primes_array[1]

@ P6
is_divided:
	@ P8 in place
	LDR R5, [R4], #4	@ primes_array[primes_counter]

	UDIV R6, R3, R5		@ Quotient
	MUL R7, R5, R6

	CMP R7, R3			@ trick: if temp == candidate, remainder == 0
	BEQ advance_candidate

@ P7
is_q_large:
	CMP R6, R5
	BHI is_divided

@ P2
candidate_is_prime:
	STR R3, [R2], #4

	@ P3
	CMP R2, R1
	BNE advance_candidate

done:
	POP {R4-R8, PC}
