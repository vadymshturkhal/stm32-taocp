.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_perform_circular_list_operations_integrate
	.type asm_perform_circular_list_operations_integrate, %function

@ With Flamboyant Exit using R6: Single-Entry Single-Exit (SESE) control flow pattern, single unified cleanup for all exit paths
@ With PTR and AVAIL caching
@ With Loop Peeling
@ With Topological Slice

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
asm_perform_circular_list_operations_integrate:
	PUSH {R4-R9, LR}

	MOVS R6, #0			@ for return synchronization

	CMP R0, #0
	BEQ set_false_return_value

	MOVS R4, R0			@ R4 = max_nodes

take_memory:
	@ void* c_circular_list_memory = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList))
	@ if (c_circular_list_memory == NULL) return 0
	@ 8 bytes for AAPCS 8-byte alignment: R4 + R5 + R6 + LR + 8=24
	LSL R0, #3
	ADDS R0, R0, #CIRCULAR_SIZE
	BL asm_balloc

	CMP R0, #0
	BEQ set_false_return_value

create_circular_list:
	@ CircularList* circular_list = c_create_circular_list(c_circular_list_memory, max_nodes)
	@ R0 is already memory
	MOVS R1, R4			@ R1 = max_nodes
	BL asm_create_circular_list_lipski_mod4
	@ R0 = circular_list

save_circular_list_and_set_loop_counter:
	MOVS R5, R0			@ R5 = circular_list
	MOVS R6, R4			@ R6 = max_nodes loop counter

insert_first_node:
	LDR R2, [R0, #CIRCULAR_AVAIL]	@ R2 = Avail
	CMP R2, #0
	BEQ set_false_return_value

	LDR R3, [R2, #NODE_LINK]		@ R3 = Avail->link;
	STR R6, [R2, #NODE_INFO]		@ P->info = info
	STR R2, [R2, #NODE_LINK]		@ P->link = P

	MOVS R7, R2						@ PTR cache
	MOVS R2, R3						@ Avail cache

	SUBS R6, R6, #1
	CBZ R6, synchronize_insert_left

.balign 4
insert_left_loop:
	CBZ R2, synchronize_insert_left_and_error_exit

	LDR R3, [R2, #NODE_LINK]		@ R3 = Avail->link;
	STR R6, [R2, #NODE_INFO]		@ P->info = info

	@ insert_p_at_front
	LDR R1, [R7, #NODE_LINK]		@ R1 = ptr->link
	STR R1, [R2, #NODE_LINK]		@ P->link = circular_list->ptr->link
	STR R2, [R7, #NODE_LINK]		@ circular_list->ptr->link = P;

	MOVS R2, R3						@ Avail = Avail->link

	SUBS R6, R6, #1
	BNE insert_left_loop

synchronize_insert_left:
	STR R2, [R0, #CIRCULAR_AVAIL]
	STR R7, [R0, #CIRCULAR_PTR]

	@ jump over synchronize_and_error_exit
	B topological_slice_pop

synchronize_insert_left_and_error_exit:
	STR R2, [R0, #CIRCULAR_AVAIL]
	STR R7, [R0, #CIRCULAR_PTR]
	B set_false_return_value

@ Topological Slice Pop
topological_slice_pop:
	MOVS R1, R4			@ R1 = max_nodes loop counter
	BL asm_circular_list_topological_slice_pop

	CBZ R0, set_false_return_value
	MOVS R0, R5			@ R0 = Circular List

insert_right_first_node:
	MOVS R6, R4

	LDR R2, [R0, #CIRCULAR_AVAIL]	@ R2 = Avail
	CBZ R2, set_false_return_value

	LDR R3, [R2, #NODE_LINK]		@ R3 = Avail->link;
	STR R6, [R2, #NODE_INFO]		@ P->info = info
	STR R2, [R2, #NODE_LINK]		@ P->link = P

	MOVS R7, R2						@ PTR cache
	MOVS R2, R3						@ Avail cache

	SUBS R6, R6, #1
	CBZ R6, synchronize_insert_right

.balign 4
insert_right_loop:
	CBZ R2, synchronize_insert_right_and_error_exit

	LDR R3, [R2, #NODE_LINK]		@ R3 = Avail->link;
	STR R6, [R2, #NODE_INFO]		@ P->info = info

	@ insert_p_at_front
	LDR R1, [R7, #NODE_LINK]		@ R1 = ptr->link
	STR R1, [R2, #NODE_LINK]		@ P->link = circular_list->ptr->link
	STR R2, [R7, #NODE_LINK]		@ circular_list->ptr->link = P;

	MOVS R7, R2						@ circular_list->ptr = P
	MOVS R2, R3						@ Avail = Avail->link

	SUBS R6, R6, #1
	BNE insert_right_loop

synchronize_insert_right:
	STR R2, [R0, #CIRCULAR_AVAIL]
	STR R7, [R0, #CIRCULAR_PTR]

	@ jump over synchronize_and_error_exit
	B clear_circular_list

synchronize_insert_right_and_error_exit:
	STR R2, [R0, #CIRCULAR_AVAIL]
	STR R7, [R0, #CIRCULAR_PTR]
	B set_false_return_value

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

error_exit:
	MOVS R0, R6
	POP {R4-R9, PC}
