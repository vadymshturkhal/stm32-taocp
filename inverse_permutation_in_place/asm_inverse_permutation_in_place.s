.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_inverse_permutation_in_place


@ Input:
@ R0: uint32_t *permutation;
@ R1: uint32_t permutation_length;

@ Runtime:
@ R0: uint32_t *permutation;
@ R1: uint32_t permutation_length then m;
@ R2: int32_t j = -1;
@ R3: int32_t i;
asm_inverse_permutation_in_place:
	@ I1 (Initialize)
	SUB R1, R1, #1
	MOV R2, #-1

main_loop:
	@ I2 (Next element)
	LDR R3, [R0, R1, LSL #2]

	@ Go to I5 (the element has already been processed)
	CMP R3, #0
	BLT after_inner_loop

@ I3
inner_loop:
	STR R2, [R0, R1, LSL #2]	@ permutation[m] = j;

	@ Reverse Subtract
	RSB R2, R1, #0				@ j = -m;

	MOV R1, R3					@ m = i;
	LDR R3, [R0, R1, LSL #2]	@ i = permutation[m];

	@ I4 (End of cycle?)
	CMP R3, #0
	BGT inner_loop

	MOV R3, R2					@ i = j

@ I5 (store final value)
after_inner_loop:
    RSB R3, R3, #0
    STR R3, [R0, R1, LSL #2]

    SUBS R1, R1, #1
    BGT main_loop

done:
	BX LR
