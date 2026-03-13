.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_find_the_original_index3

@ Input:
@ R0 char *original_order
@ R1 uint32_t original_order_length
@ R2 char element_to_find

@ Runtime:
@ R0 original_order + counter
@ R1 original_order + original_order_length
@ R2 char element_to_find
@ R3 current_element
@ R12 original_order

@ 2257 cycles
asm_find_the_original_index3:
	@ early return
	CBZ R1, not_found

	ADD R12, R0, #1

	@ Trick for avoiding SUBS in done
	ADD R1, R0, R1

find_index_loop:
	LDRB R3, [R0], #1
	CMP R3, R2
	BEQ done

	@ slower
	@ SUBS R3, R1, R0
	@ CBZ R3, not_found

	CMP R0, R1
	BNE find_index_loop

not_found:
	MOV R0, #-1
	BX LR

done:
	SUB R0, R0, R12
	BX LR
