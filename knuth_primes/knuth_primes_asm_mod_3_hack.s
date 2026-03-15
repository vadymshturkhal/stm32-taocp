.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_knuth_primes_mod_3_hack


@ using mod 3 hack filter


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
@ R2 current_prime pointer
@ R3 candidate
@ R4 traversal_primes pointer
@ R5 divisor
@ R6 Q
@ R7 temp (divisor * Q)
@ R8 0xAAAAAAAB (modular inverse of 3)
@ R9 0x55555555 (max valid limit for mod 3)


asm_knuth_primes_mod_3_hack:
	@ P1
	PUSH {R4-R9, LR}

	@ pre-load mod 3 filter constand
	LDR R8, =0xAAAAAAAB
	LDR R9, =0x55555555

	@ store 2 and 3 to primes_array
	MOVS R3, #2
	STR R3, [R0, #0]
    MOVS R3, #3
    STR R3, [R0, #4]

	ADDS R2, R0, #8			@ R2 points to primes_array[2]
	ADDS R1, R0, R1, LSL #2	@ end_pointer = primes_array + (primes_to_find * 4)

	MOVS R3, #5				@ start candidate = 5
	B candidate_is_prime	@ add 5 to primes array

@ P4
advance_candidate:
	ADDS R3, R3, #2

    @ mod 3 filter hack
    MUL R6, R3, R8
    CMP R6, R9
    IT LS
    ADDLS R3, R3, #2		@ Add 2 if it is a multiple of 3

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
	POP {R4-R9, PC}
