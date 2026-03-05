.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_auxiliary_table1

@ R4 = permutation moving pointer (starts at end)
@ R5 = original_order base
@ R6 = auxiliary_table base
@ R8 = i (length counter)
@ R9 = j
@ R10 = Z
@ R11 = permutation base pointer

@ 2060 cycles
asm_get_auxiliary_table1:
    PUSH {R4-R11, LR}

    @ 1. INITIALIZATION
    ADD R4, R0, R1
    SUB R4, R4, #1      @ R4 points to the last char of permutation

    MOV R5, R2
    MOV R6, R3
    MOV R8, #0          @ i = 0
    MOV R9, #0          @ j = 0
    MOV R10, R1         @ Z = initial value
    MOV R11, R0         @ Base pointer for loop termination

.balign 4
examine_the_next_element:
    @ Load char and decrement pointer in one cycle
    LDRB R7, [R4], #-1

    @ 2. HOT-PATH OPTIMIZATION
    @ Letters are common, parentheses are rare.
    @ We branch AWAY for parens to keep the pipeline clean for letters.
    CMP R7, ')'
    BEQ handle_close_paren
    CMP R7, '('
    BEQ handle_open_paren

    @ 3. THE INLINED SEARCH LOOP (No 'BL' instruction!)
    CMP R8, #0
    BEQ add_element_to_arrays       @ If i == 0, skip search and add

    MOV R0, R5                      @ R0 = moving search pointer
    ADD R1, R5, R8                  @ R1 = end of search pointer

inline_search_loop:
    LDRB R3, [R0], #1
    CMP R3, R7
    BEQ found_index                 @ Match found!

    CMP R0, R1
    BLO inline_search_loop          @ Keep searching

    @ If we fall through, it wasn't found
    B add_element_to_arrays

found_index:
    @ Calculate original_index (R0)
    SUB R0, R0, R5
    SUB R0, R0, #1

perform_swap:
    @ B3: Change T[i]
    LDRB R1, [R6, R0]               @ current_element = auxiliary_table[original_index]
    STRB R10, [R6, R0]              @ auxiliary_table[original_index] = Z

    CMP R10, #1
    IT EQ
    MOVEQ R9, R0                    @ if (Z == 1) j = original_index

    MOV R10, R1                     @ Z = current_element
    B loop_condition

add_element_to_arrays:
    STRB R7, [R5, R8]               @ original_order[i] = element
    STRB R7, [R6, R8]               @ auxiliary_table[i] = element
    MOV R0, R8                      @ original_index = i
    ADD R8, R8, #1                  @ i++
    B perform_swap                  @ Go do the swap

@ --- OUT-OF-LINE HANDLERS ---
@ These sit outside the hot-path so they don't clutter the instruction cache
handle_close_paren:
    MOV R10, #1                     @ Z = 1
    B loop_condition

handle_open_paren:
    STRB R10, [R6, R9]              @ auxiliary_table[j] = Z
    B loop_condition

loop_condition:
    @ 4. BOTTOM-HEAVY LOOPING
    @ Loop until R4 drops below R11 (permutation base)
    CMP R4, R11
    BHS examine_the_next_element    @ Branch if Higher or Same (unsigned >=)

done:
    MOV R0, R8                      @ Return i
    POP {R4-R11, PC}
