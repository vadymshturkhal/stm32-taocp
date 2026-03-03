.syntax unified
	.thumb
	.cpu cortex-m4
	.global first_pass_hack_asm



@ R0 permutation;
@ R1 parsed_permutation;
@ R2 permutation_length;
first_pass_hack_asm:
    PUSH {r4, lr}
    CMP r2, #0
    BEQ .L_end
    @ 1 cycle faster
    @ SUBS R3, R2, #0
    @ CBZ R3, .L_end

.balign 4
.L_loop:
    LDRB r3, [r0], #1       @ Load current_char, auto-increment source

    @ --- Condition 1: if (current_char == '(') ---
    CMP r3, #'('
    ITT EQ                  @ IF-THEN block: Next 2 instructions only run if EQual
    ORREQ r3, r3, #0x80     @ Tag it: current_char | 0x80
    LDRBEQ r4, [r0]         @ cycle_begin = permutation[i + 1] (r0 is already at i+1!)

    @ --- Condition 2: else if (current_char == ')') ---
    @ Note: If it WAS '(', r3 is now 0xA8, so this CMP will safely fail.
    CMP r3, #')'
    ITT EQ                  @ IF-THEN block: Next 2 instructions only run if EQual
    MOVEQ r3, r4            @ current_char = cycle_begin
    ORREQ r3, r3, #0x80     @ Tag it: current_char | 0x80

    @ --- Store and Loop ---
    STRB r3, [r1], #1       @ Store the result, auto-increment destination
    SUBS r2, r2, #1         @ Decrement length
    BNE .L_loop             @ Loop back if length > 0

.L_end:
    MOV r0, r1              @ Return original destination pointer
    POP {r4, pc}
