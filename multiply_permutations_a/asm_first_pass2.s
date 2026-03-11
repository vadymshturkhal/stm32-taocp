.syntax unified
    .thumb
    .cpu cortex-m4
    .global first_pass_hack_asm2


@ Using tail duplication

@ R0 permutation;
@ R1 parsed_permutation;
@ R2 permutation_length;
@ R3 current_char;
first_pass_hack_asm2:
    PUSH {R4, LR}

    @ If length is 0, jump immediately to done
    CBZ R2, done

.balign 4
main_loop:
    LDRB R3, [R0], #1       @ Load char, auto-increment

    CMP R3, #'('
    BEQ left_equal          @ Jump out of hot path (1 cycle if false)

    CMP R3, #')'
    BEQ right_equal         @ Jump out of hot path (1 cycle if false)

    @ Hot path
    STRB R3, [R1], #1       @ Store the result
    SUBS R2, R2, #1         @ Decrement length
    BNE main_loop           @ Loop back
    B done

.balign 4
left_equal:
    ORR R3, R3, #0x80       @ Tag it
    LDRB R4, [R0]           @ cycle_begin = permutation[i + 1]

    @ Tail duplication
    STRB R3, [R1], #1
    SUBS R2, R2, #1
    BNE main_loop           @ Jump straight back to the TOP, no double branch!
    B done

.balign 4
right_equal:
    MOV R3, R4              @ current_char = cycle_begin
    ORR R3, R3, #0x80       @ Tag it

    @ Tail duplication!
    STRB R3, [R1], #1
    SUBS R2, R2, #1
    BNE main_loop           @ Jump straight back to the TOP, no double branch!

done:
    MOV R0, R1              @ Return original destination pointer
    POP {R4, PC}
