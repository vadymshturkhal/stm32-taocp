.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_inverse_permutation_in_place_j


@ Input:
@ R0: uint32_t *permutation;
@ R1: uint32_t permutation_length;

@ Runtime:
@ R0 uint32_t *permutation;
@ R1 uint32_t permutation_length then m;
@ R2 int32_t j;
@ R3 int32_t i;
@ R12 temp: end boundary and permutation[-i];
asm_inverse_permutation_in_place_j:
	@ start negation
	ADDS R2, R0, #4				@ start from permutation[1]
	ADDS R12, R0, R1, LSL #2	@ end boundary (pointer + length * 4)

@ J1 (Negate all)
negate_all_loop:
	LDR R3, [R2]
	NEGS R3, R3
	STR R3, [R2], #4

	CMP R2, R12
	BLO negate_all_loop

	SUBS R1, R1, #1		@ m = permutation_length - 1;

@ J2 (Initialize)
initialize:
	MOVS R2, R1			@ j = m

@ J3 (Find negative entry)
@ Set i = permutations[j], if i > 0: j = i and repeat
find_negative_entry:
	LDR R3, [R0, R2, LSL #2]	@ i = permutations[j]
	CMP R3, #0
	BLE invert

find_j:
	MOVS R2, R3					@ if i > 0: j = i
	LDR R3, [R0, R2, LSL #2]
	CMP R3, #0
	BGT find_j

@ J4 (Invert)
@ Set permutation[j] = permutation[-i], permutation[-i] = m
invert:
	NEGS R3, R3
	LDR R12, [R0, R3, LSL #2]	@ permutation[-i]

	STR R12, [R0, R2, LSL #2]	@ permutation[j] = permutation[-i]
	STR R1, [R0, R3, LSL #2]	@ permutation[-i] = m

	@ J5 (Loop on m)
	SUBS R1, R1, #1				@ Decrese m by 1
	BNE initialize				@ if m > 0, go back to J2, terminate otherwise

done:
	BX LR
