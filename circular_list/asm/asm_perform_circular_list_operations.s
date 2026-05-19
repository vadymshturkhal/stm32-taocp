.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_perform_circular_list_operations
	.type asm_perform_circular_list_operations, %function

@ With Flamboyant Exit using R6: Single-Entry Single-Exit (SESE) control flow pattern, single unified cleanup for all exit paths

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
.equ CIRCULAR_AVAIL,4
.equ CIRCULAR_SIZE, 8

@ Input:
@ R0 max_nodes

@ Runtime:
@ R0 max_nodes, circular_list_memory, circular_list, pop_is_success
@ R1 max_nodes, circular_list
@ R2 max_nodes
@ R4 max_nodes
@ R5 circular_list
@ R6 return_flag, max_nodes loop counter

@ Return 0 or 1
asm_perform_circular_list_operations:
	PUSH {R4-R6, LR}

	MOVS R6, #0			@ for return synchronization
	CBZ R0, error_exit

	MOVS R4, R0			@ R4 = max_nodes

take_memory:
	@ void* c_circular_list_memory = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList))
	@ if (c_circular_list_memory == NULL) return 0
	@ 8 bytes for AAPCS 8-byte alignment: R4 + R5 + R6 + LR + 8=24
	LSL R0, #3
	ADDS R0, R0, #CIRCULAR_SIZE
	BL asm_balloc

	CBZ R0, error_exit

set_success_flag_on_stack:
	@ bool pop_is_success = true
	SUB SP, SP, #8
	MOVS R5, #1
	STRB R5, [SP, #0]

create_circular_list:
	@ CircularList* circular_list = c_create_circular_list(c_circular_list_memory, max_nodes)
	@ R0 is already memory
	MOVS R1, R4			@ R1 = max_nodes
	BL asm_create_circular_list_lipski_mod4
	@ R0 = circular_list

save_circular_list_and_set_loop_counter:
	MOVS R5, R0			@ R5 = circular_list
	MOVS R6, R4			@ R6 = max_nodes loop counter

.balign 4
insert_left_loop:
	MOVS R0, R5			@ R0 = circular_list
	MOVS R1, R6			@ R1 = i

	BL asm_circular_list_insert_left
	CBZ R0, set_false_return_value

	SUBS R6, R6, #1
	BNE insert_left_loop

set_loop_counter:
	MOVS R6, R4			@ R6 = max_nodes loop counter

.balign 4
pop_loop:
	MOV R0, SP			@ R0 = bool pop_is_success
	MOVS R1, R5			@ R1 = circular_list
	BL asm_circular_list_pop

	@ R0 = info

	LDRB R1, [SP]
	CBZ R1, set_false_return_value

	SUBS R6, R6, #1
	BNE pop_loop

@ use R4 instead of init R6
.balign 4
insert_right_loop:
	MOVS R0, R5			@ R0 = circular_list
	MOVS R1, R4			@ R1 = i
	BL asm_circular_list_insert_right_inline

	CBZ R0, set_false_return_value

	SUBS R4, R4, #1
	BNE insert_right_loop

clear_circular_list:
	MOVS R0, R5			@ R0 = circular_list
	BL asm_circular_list_clear

@ Flamboyant Exit
set_success_return_value:
	MOVS R6, #1
	B free_memory

set_false_return_value:
	MOVS R6, #0

free_memory:
	MOVS R0, R5			@ R1 = circular_list
	BL asm_balloc_free

	@ remove pop_is_success from stack
	ADD SP, SP, #8

error_exit:
	MOVS R0, R6
	POP {R4-R6, PC}
