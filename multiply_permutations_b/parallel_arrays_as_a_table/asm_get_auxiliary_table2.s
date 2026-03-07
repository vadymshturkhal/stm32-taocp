.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_auxiliary_table2


@ Store i, j, Z to R8

@ R4  = permutation moving pointer
@ R5  = original_order base
@ R6  = auxiliary_table base
@ R8  = PACKED REGISTER: [ Unused (8) | Z (8) | j (8) | i (8) ]
@ R11 = permutation base pointer

@ 2115 cycles
asm_get_auxiliary_table2:
    @ Clean 6-register push. (Saves 6 cycles over the unpacked version!)
    PUSH {R4-R6, R8, R11, LR}

    @ 1. INITIALIZATION
    ADD R4, R0, R1
    SUB R4, R4, #1

    MOV R5, R2
    MOV R6, R3
    MOV R11, R0

    @ Pack initial Z (from R1) into R8.
    @ Shifts Z left by 16, naturally zeroing out i and j.
    LSL R8, R1, #16

.balign 4
examine_the_next_element:
    @ FIX: Use R2 instead of R7 so we don't clobber the C compiler!
    LDRB R2, [R4], #-1
    MOV R0, R5                  @ Free instruction to hide load stall

    CMP R2, ')'
    BEQ handle_close_paren
    CMP R2, '('
    BEQ handle_open_paren

    @ Extract i (lowest 8 bits) into R3
    UXTB R3, R8
    CBZ R3, add_element_to_arrays

    @ Setup search pointer using extracted i (R3)
    ADD R1, R5, R3

inline_search_loop:
    LDRB R3, [R0], #1
    CMP R3, R2                  @ Compare with R2!
    BEQ found_index

    CMP R0, R1
    BLO inline_search_loop

    B add_element_to_arrays

found_index:
    SUB R0, R0, R5
    SUB R0, R0, #1

perform_swap:
    @ B3: Change T[i]
    UBFX R3, R8, #16, #8        @ Extract Z
    LDRB R1, [R6, R0]
    STRB R3, [R6, R0]

    CMP R3, #1
    IT EQ
    BFIEQ R8, R0, #8, #8        @ If Z==1, insert original_index into j

    BFI R8, R1, #16, #8         @ Insert new Z into Z
    B loop_condition

add_element_to_arrays:
    @ Extract i into R3
    UXTB R3, R8
    STRB R2, [R5, R3]           @ Store R2!
    STRB R2, [R6, R3]           @ Store R2!
    MOV R0, R3

    @ Increment i!
    ADD R8, R8, #1
    B perform_swap

handle_close_paren:
    MOV R3, #1
    BFI R8, R3, #16, #8         @ Insert 1 into Z
    B loop_condition

handle_open_paren:
    UBFX R3, R8, #16, #8        @ Extract Z
    UBFX R12, R8, #8, #8        @ Extract j
    STRB R3, [R6, R12]          @ auxiliary_table[j] = Z
    B loop_condition

loop_condition:
    CMP R4, R11
    BHS examine_the_next_element

done:
    UXTB R0, R8                 @ Return i
    POP {R4-R6, R8, R11, PC}
