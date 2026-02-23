.syntax unified
	.thumb
	.cpu cortex-m4
	.global asm_knuth_primes


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


@ Registers
@ R0 primes_array
@ R1 primes_to_find
@ R2 primes_found
@ R3 candidate
@ R4 primes_counter
@ R5 primes_array[primes_counter]
@ R6 Q
@ R7 R


asm_knuth_primes:
	@ P1
	PUSH {R4-R7, LR}
	MOV R3, #2
	STR R3, [R0]
	MOV R2, #1
	MOV R3, #3
	MOV R4, #0

@ P2
candidate_is_prime:
	STR R3, [R0, R2, LSL #2]
	ADD R2, R2, #1

	@ P3
	CMP R2, R1
	BEQ done

@ P4
advance_candidate:
	ADD R3, R3, #2

@ P5
init_primes_counter:
	MOV R4, #1

@ P6
is_divided:
	@ primes_array[primes_counter]
	LDR R5, [R0, R4, LSL #2]

	@ Q
	UDIV R6, R3, R5

	@ R
	MLS R7, R5, R6, R3

	CMP R7, #0
	BEQ advance_candidate

@ P7
is_q_large:
	CMP R6, R5
	BLS candidate_is_prime

@ P8
increase_primes_counter:
	ADD R4, #1
	B is_divided

done:
	POP {R4-R7, PC}
