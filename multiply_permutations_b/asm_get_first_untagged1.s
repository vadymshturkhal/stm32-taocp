.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_first_untagged1


@ Searching from left to right,
@ find the first untagged element of the original_order
@ and return its index
@ else return -1

@ Input:
@ R0 char *original_order
@ R1 uint32_t original_order_length

@ R2 original_order + 1
@ R3 current_char
asm_get_first_untagged1:
	@ MOV R2, R0

	@ 2141
	@ Trick for avoiding SUBS R0, R0, #1 in return_i
	ADD R2, R0, #1

	ADD R1, R0, R1

.balign 4
search_first_untagged:
	@ parsed_permutation[i]
	LDRB R3, [R0], #1

	@ TST does a bitwise AND and sets flags, without modifying R3!
    @ If the top bit is 0 (untagged), the Zero (Z) flag is set.
    @ parsed_permutation[i] & 0x80 == 0
    @ 2146
	TST R3, 0x80
	BEQ done

	@ 2163
	@AND R3, R3, 0x80
	@CBZ R3, return_i

	@ 2163
	@AND R3, R3, 0x80
	@CBZ R3, return_i

	CMP R0, R1
	BLO search_first_untagged

	@ SUBS R1, R1, #1
	@ BNE search_first_untagged

not_found:
	MOV R0, #-1
	BX LR

done:
	SUB R0, R0, R2

	@ Branch and Exchange to Link Register
	BX LR
