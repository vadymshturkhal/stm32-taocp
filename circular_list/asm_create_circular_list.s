.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_create_circular_list
	.type asm_create_circular_list, %function

@ Used Lipski Trick (MVE mod 2)
@ ~5.6 cycles per node

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ PTR, 			0
.equ CIRCULAR_AVAIL,4
.equ CIRCULAR_SIZE, 8

@ Input:
@ R0 c_circular_list_memory
@ R1 max_nodes

@ Runtime:
@ R0 c_circular_list_memory
@ R1 max_nodes
@ R2 NULL or 0, tmp
@ R3 Avail

asm_create_circular_list:
	MOVS R2, #0
	STR R2, [R0, #PTR]	@ circular_list->ptr = NULL
	CBZ R1, early_exit

.balign 4
init_storage_pool:
	ADDS R3, R0, #CIRCULAR_SIZE	@ (CircularNode*)(circular_list + 1)
	STR R2, [R3, #NODE_LINK]	@ avail->link = NULL

	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done				@ if there is only 1 node

.balign 4
linking_loop:
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail

	@ MOVS R3, R2					@ avail = tmp
	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done

	ADDS R3, R2, #NODE_SIZE		@ tmp = avail+1
	STR R2, [R3, #NODE_LINK]	@ tmp->link = avail

	@ MOVS R2, R3					@ avail = tmp
	SUBS R1, R1, #1				@ max_nodes--
	BNE linking_loop

sync_avail:
	MOVS R2, R3

done:
	STR R2, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = avail
	BX LR

early_exit:
	STR R2, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = NULL
	BX LR
