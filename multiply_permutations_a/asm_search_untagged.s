.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_search_untagged


@ Input:
@ R0 uint8_t *parsed_permutation;
@ R1 uint32_t permutation_length;

@ Runtime:
@ R0 uint8_t *parsed_permutation;
@ R1 uint32_t permutation_length;
@ R2 parsed_permutation[i];
@ R3 parsed_permutation + 1;

@ Return:
@ index or 0;

asm_search_untagged:
	@ save the start of the parsed_permutation
	ADDS R3, R0, #1		@ pointer trick names Pointer Biasing

.balign 4
search_loop:
	@ parsed_permutation[i]
	LDRB R2,[R0], #1

	@ parsed_permutation[i] & 0x80 == 0
	@AND R2, R2, 0x80
	@CMP R2, #0
	@BEQ return_i

	@ TST does a bitwise AND and sets flags, without modifying R2!
    @ If the top bit is 0 (untagged), the Zero (Z) flag is set.
    @ parsed_permutation[i] & 0x80 == 0
	TST R2, 0x80
	BEQ return_i

	SUBS R1, R1, #1
	BNE search_loop

not_found:
	MOVS R0, #0
	BX LR

return_i:
	SUBS R0, R0, R3
	BX LR	@ Branch and Exchange to Link Register
