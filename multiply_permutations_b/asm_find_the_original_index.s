.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_find_the_original_index

@ Input:
@ R0 char *original_order
@ R1 uint32_t original_order_length
@ R2 char element_to_find

@ Runtime:
@ R0 original_order
@ R1 original_order + original_order_length
@ R2 char element_to_find
@ R3 current_element
@ R4 original_order + counter

@ 2483 cycles
asm_find_the_original_index:
	CBZ R1, early_return

	PUSH {R4, LR}
	MOV R4, R0
	ADD R1, R0, R1

	@ Trick for avoiding SUBS in done
	ADD R0, R0, #1

.balign 4
find_index_loop:
	LDRB R3, [R4], #1
	CMP R3, R2
	BEQ done

	@ very slow
	@ SUBS R1, R1, #1
	@ CBZ R1, not_found

	CMP R4, R1
	BEQ not_found

	B find_index_loop

early_return:
	MOV R0, #-1
	BX LR

not_found:
	MOV R0, #-1
	POP {R4, PC}

done:
	SUB R0, R4, R0
	POP {R4, PC}
