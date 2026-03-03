.syntax unified
	.thumb
	.cpu cortex-m4
	.global first_pass_hack_asm1


@ R0 permutation;
@ R1 parsed_permutation;
@ R2 permutation_length;
first_pass_hack_asm1:
    PUSH {R4, LR}
    SUBS R3, R2, #0
    CBZ R3, done

.balign 4
main_loop:
	@ Load current_char and auto-increment source
    LDRB R3, [R0], #1

	@ if (current_char == '(')
	CMP R3, #'('
	BEQ left_equal

    @ if (current_char == ')')
    CMP r3, #')'
    ITT EQ                  @ IF-THEN block: Next 2 instructions only run if EQual
    MOVEQ r3, r4            @ current_char = cycle_begin
    ORREQ r3, r3, #0x80     @ Tag it: current_char | 0x80

store_and_loop:
    @ --- Store and Loop ---
    STRB R3, [R1], #1       @ Store the result, auto-increment destination
    SUBS R2, R2, #1         @ Decrement length
    BNE main_loop             @ Loop back if length > 0
    B done

.balign 4
left_equal:
    ORR R3, R3, #0x80     @ Tag it: current_char | 0x80
    LDRB R4, [R0]         @ cycle_begin = permutation[i + 1] (r0 is already at i+1!)
	B store_and_loop

done:
    MOV R0, R1              @ Return original destination pointer
    POP {R4, PC}
