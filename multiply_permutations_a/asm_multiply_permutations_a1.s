.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_multiply_permutations_a1


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 char *multiplication_result;

@ Runtime:
@ R4 char *permutation;
@ R5 uint32_t permutation_length;
@ R6 char *multiplication_result;
@ R7 stack save;

asm_multiply_permutations_a1:
	PUSH {R4-R7, LR}

	@ save parameters
	MOV R4, R0
	MOV R5, R1
	MOV R6, R2

 	@ on stack char parsed_permutation[permutation_length];
	ADD R1, R5, #7
	BIC R1, R1, #7

	SUB SP, SP, R1	@ memory allocated on the stack
	MOV R7, R1		@ save the allocated size

	@ R0 permutation at the moment;
	MOV R1, SP	@ R1 parsed_permutation;
	MOV R2, R5	@ R2 permutation_length;

 	@ A1
 	BL first_pass_hack_asm2	@ first_pass_hack_asm2(permutation, parsed_permutation, permutation_length);

@ A2
search_untagged:
	MOV R0, SP
	MOV R1, R5
	BL asm_search_untagged

	@ R0 contains the index of the untagged char
	@ if R0 == 0 we are done
	CBZ R0, done

    @ R0 holds our valid start_index.
    MOV R4, R0              @ start_index = R0

    @ 1. multiplication_result[multiplication_result_length] = '(';
    MOVS R3, #'('
    STRB R3, [R6], #1	@ 2. multiplication_result_length++;

    @ 3. start = parsed_permutation[start_index];
    LDRB R3, [SP, R4]	@ R4 (start) = parsed_permutation[start_index]

    @ 4. multiplication_result[multiplication_result_length] = start;
    STRB R3, [R6], #1	@ 5. multiplication_result_length++;

    @ 6. parsed_permutation[start_index] |= 0x80;
    ORR R3, R3, #0x80       @ Tag the character
    STRB R3, [SP, R4]          @ Write it back to the array

    MOV R0, SP	@ Arg 1: parsed_permutation
    MOV R1, R5	@ Arg 2: permutation_length
    MOV R2, R6	@ Arg 3: multiplication_result
    MOV R3, R4	@ Arg 4: start_index
    BL asm_find_cycle_hack4

    MOV R6, R0			@ Update multiplication_result
    B search_untagged	@ Loop back to search for the next untagged character!

done:
	@ multiplication_result[multiplication_result_length] = '\0';
	MOVS R3, #0
	STRB R3, [R6]

	@ restore SP
	ADD SP, SP, R7
	POP {R4-R7, PC}
