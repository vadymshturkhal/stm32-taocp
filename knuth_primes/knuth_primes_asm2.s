.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_knuth_primes2


@ Algorithm
@ P1
@ init primes_array[0] = 2;
@ init primes_found = 1;
@ init candidate = 3;
@ init primes_counter = 0;

@ P2
@ candidate is_prime:
	@ primes_array[primes_found] = candidate;
	@ primes_found += 1;

@ P3
@ If primes_found == primes_to_find - 1: return

@ P4
@ candidate += 2;

@ P5
@ primes_counter = 1. (will run through the possible prime divisors of candidate);

@ P6
@ primes_array[primes_counter] divides candidate?
@ divide candidate by primes_array[primes_counter] and let Q = quotient, R = remainder;
	@ if R == 0: N is not prime => go to P4;

@ P7
@ primes_array[primes_counter] large?
@ If Q <= primes_array[primes_counter], go to P2. (in such case candidate must be prime)

@ P8
@ primes_counter += 1 and go to P6;


@ Input:
@ R0 primes_array
@ R1 primes_to_find

@ Runtime:
@ R1 end pointer
@ R2 currebt_prime pointer
@ R3 candidate
@ R4 traversal_primes pointer
@ R5 divisor
@ R6 Q
@ R7 temp (divisor * Q)


asm_knuth_primes2:
	@ P1
	PUSH {R4-R7, LR}

	@ store 2 to primes_array
	MOVS R3, #2
	STR R3, [R0]

	ADDS R2, R0, #4			@ primes_array pointer += 4
	ADDS R1, R0, R1, LSL #2	@ end_pointer = primes_array + (primes_to_find * 4)

	MOVS R3, #3				@ candidate = 3

	B candidate_is_prime	@ add 3 to primes array

@ P4
.balign 4
advance_candidate:
	ADDS R3, R3, #2

@ P5
init_primes_counter:
	ADDS R4, R0, #4

@ P6
.balign 8
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
	@ BLS candidate_is_prime
	BHI is_divided

@ P2
candidate_is_prime:
	STR R3, [R2], #4

	@ P3
	CMP R2, R1
	BNE advance_candidate

done:
	POP {R4-R7, PC}
