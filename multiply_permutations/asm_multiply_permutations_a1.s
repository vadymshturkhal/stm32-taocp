.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_multiply_permutations_a1


@ char *permutation, uint32_t permutation_length, char *multiplication_result
@ start
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 char *multiplication_result;
@ A1

@ runtime
@ R4 char *permutation;
@ R5 uint32_t permutation_length;
@ R6 char *multiplication_result;
@ R7 stack save;
@ R8 multiplication_result_length;
@ R9 start;
asm_multiply_permutations_a1:
	@ Push 8 registers (32 bytes) to keep the stack perfectly 8-byte aligned
	PUSH {R4-R10, LR}

	@ save parameters
	MOV R4, R0
	MOV R5, R1
	MOV R6, R2

 	@ on stack char parsed_permutation[permutation_length];
	ADD R12, R5, #7
	BIC R12, R12, #7

	@ memory allocated on the stack
	SUB SP, SP, R12
	@ save the allocated size
	MOV R7, R12

	@ R0 permutation;
	@ R1 parsed_permutation;
	@ R2 permutation_length;
	MOV R1, SP
	MOV R2, R5

 	@ A1
 	@ first_pass_hack_asm2(permutation, parsed_permutation, permutation_length);
 	BL first_pass_hack_asm2

	@ uint32_t multiplication_result_length = 0;
	@ FIXME useless
	MOV R8, #0;

@ A2
@ R0 uint8_t *parsed_permutation;
@ R1 uint32_t permutation_length;
search_untagged:
	MOV R0, SP
	MOV R1, R5

	BL asm_search_untagged

	@ R0 contains the index of the untagged char
	@ if R0 == 0 we are done
	CBZ R0, done

@ --- WE ARE RIGHT AFTER: CBZ R0, done ---
    @ R0 holds our valid start_index.
    MOV R9, R0              @ start_index = R0

    @ 1. multiplication_result[multiplication_result_length] = '(';
    MOVS R3, #'('
    @ x STRB R3, [R6, R8]       @ Store '(' at base (R6) + length (R8)
    STRB R3, [R6], #1

    @ 2. multiplication_result_length++;
    ADD R8, R8, #1

    @ 3. start = parsed_permutation[start_index];
    ADD R10, SP, R9         @ R10 = &parsed_permutation[start_index]
    LDRB R4, [R10]          @ R4 (start) = parsed_permutation[start_index]

    @ 4. multiplication_result[multiplication_result_length] = start;
    @ x STRB R4, [R6, R8]       @ Store 'start' char at base (R6) + length (R8)
    STRB R4, [R6], #1

    @ 5. multiplication_result_length++;
    ADD R8, R8, #1

    @ 6. parsed_permutation[start_index] |= 0x80;
    ORR R3, R4, #0x80       @ Tag the character
    STRB R3, [R10]          @ Write it back to the array

	@ Arg 1: parsed_permutation
    MOV R0, SP

    @ Arg 2: permutation_length
    MOV R1, R5

    @ Arg 3: multiplication_result
    MOV R2, R6

    @ Arg 4: start_index
    MOV R3, R9

    BL asm_find_cycle_hack4

    @ Update multiplication_result
    MOV R6, R0

    @ Loop back to search for the next untagged character!
    B search_untagged

done:
	@ multiplication_result[multiplication_result_length] = '\0';
	MOVS R3, #0
	@ x STRB R3, [R6, R8]
	STRB R3, [R6]

	@ restore SP
	ADD SP, SP, R7
	POP {R4-R10, PC}
