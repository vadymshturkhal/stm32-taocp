.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_get_auxiliary_table5


@ some tricks using:
@ Hot/Cold Path Isolation (Cache Saturation);
@ Zero-Cost Offset Absorption;
@ The SRAM Bypass;
@ Branchless Conditional Execution;
@ Extreme Tail Duplication;
@ Surgical Mod 16 Cache Alignment;


@ Input:
@ R0 char *permutation;
@ R1 uint32_t permutation_length;
@ R2 char *original_order;
@ R3 char *auxiliary_table;

@ Runtime:
@ R3 char current element;
@ R4 permutation + permutation_length - 1;
@ R5 char *original_order base;
@ R8 char *auxiliary_table base;
@ R7 char element;
@ R6 uint32_t i (length counter);
@ R9 uint32_t j;
@ R2 char Z;
@ R10 char *permutation;


asm_get_auxiliary_table5:
    PUSH {R4-R10, LR}

    @ Init
    ADD R4, R0, R1
    SUB R4, R4, #1      @ R4 points to the last char of permutation

    MOVS R5, R2
    MOV R8, R3
	MOVS R6, #0        	@ i = 0
    MOV R9, #0          @ j = 0
    MOVS R2, R1         @ Z = initial value
    MOV R10, R0         @ Base pointer for loop termination

	@ 3xMOVS + 2 bytes
    NOP

    @ Pre-calculate (Base Pointer + 1)
    ADD R12, R5, #1

examine_the_next_element:
    LDRB R7, [R4], #-1

    @ handle parenteses
    CMP R7, #97                     @ ASCII 'a' is 97
    BLO handle_parentheses          @ If strictly less than 97, it's a parenthesis! Jump out!

    @ letters hot path
    CBZ R6, add_element_to_arrays

    MOV R0, R5
    ADD R1, R5, R6

inline_search_loop:
    LDRB R3, [R0], #1
    CMP R3, R7
    BEQ found_index                 @ Match found!

    CMP R0, R1
    BLO inline_search_loop          @ Keep searching

    @ If we fall through, it wasn't found

add_element_to_arrays:
    STRB R7, [R5, R6]               @ original_order[i] = element
    MOVS R0, R6                     @ original_index = i
    ADDS R6, R6, #1                 @ i++

    @ B3: Change T[i]
    MOVS R1, R7
    STRB R2, [R8, R0]              	@ auxiliary_table[original_index] = Z

	CBZ R2, update_j_and_z			@ if (Z == 0) j = original_index

    MOVS R2, R1                     	@ Z = current_element
    B loop_condition

    @ surgical alightment
    NOP								@ 2 bytes of padding
    NOP
    NOP

found_index:
    @ Calculate original_index (R0) in ONE instruction instead of two!
    @ R0 = Current Pointer - (Base Pointer + 1)
    SUBS R0, R0, R12

perform_swap:
    @ B3: Change T[i]
    LDRB R1, [R8, R0]               @ current_element = auxiliary_table[original_index]
    STRB R2, [R8, R0]              	@ auxiliary_table[original_index] = Z

    CBZ R2, update_j_and_z			@ if (Z == 0) j = original_index

    MOV R2, R1                     	@ Z = current_element

	@ tail duplication
    @ B loop_condition
    CMP R4, R10
    BHS examine_the_next_element    @ Branch if Higher or Same (unsigned >=)

@ These sit outside the hot-path so they don't clutter the instruction cache
handle_close_parenthesis:
    MOV R2, #0                     @ Z = 0

	@ tail duplication
    @ loop_condition
    CMP R4, R10
    BHS examine_the_next_element    @ Branch if Higher or Same (unsigned >=)

	@ B done
	MOV R0, R6                     @ Return i
    POP {R4-R10, PC}

update_j_and_z:
    MOV R9, R0                   	@ if (Z == 0) j = original_index
    MOV R2, R1                     	@ Z = current_element

	@ tail duplication
    @ B loop_condition
    CMP R4, R10
    BHS examine_the_next_element    @ Branch if Higher or Same (unsigned >=)

    @ done:
    MOV R0, R6                     @ Return i
    POP {R4-R10, PC}

handle_parentheses:
    CMP R7, #41                     @ ASCII ')' is 41
    BEQ handle_close_parenthesis	@ If it's not ')', it MUST be '('

handle_open_parenthesis:
    STRB R2, [R8, R9]              @ auxiliary_table[j] = Z

loop_condition:
    CMP R4, R10
    BHS examine_the_next_element    @ Branch if Higher or Same (unsigned >=)

done:
    MOV R0, R6                     @ Return i
    POP {R4-R10, PC}
