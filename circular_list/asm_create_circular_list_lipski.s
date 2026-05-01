.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_create_circular_list_lipski
	.type asm_create_circular_list_lipski, %function

@ Used Lipski Trick (MVE mod 2) with hoisting, Duff's Device, and pre-calculating tmp node
@ ~4.25 cycles per node

@ Struct memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

.equ CIRCULAR_PTR, 	0
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

asm_create_circular_list_lipski:
	MOVS R2, #0
	STR R2, [R0, #CIRCULAR_PTR]	@ circular_list->ptr = NULL
	CBZ R1, early_exit

.balign 4
init_storage_pool:
	ADDS R3, R0, #CIRCULAR_SIZE	@ (CircularNode*)(circular_list + 1)
	STR R2, [R3, #NODE_LINK]	@ avail->link = NULL

	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done				@ if there is only 1 node

	@ check mod 2 (Duff's Device)
	TST R1, #1					@ if max_nodes is even jump to hoist_tmp
	BEQ hoist_tmp

	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail

	MOVS R3, R2					@ avail = tmp
	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done

hoist_tmp:
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1

.balign 4
linking_loop:
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail
	ADDS R3, R2, #NODE_SIZE		@ tmp = avail+1

	STR R2, [R3, #NODE_LINK]	@ tmp->link = avail
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1

	SUBS R1, R1, #2				@ max_nodes -= 2
	BNE linking_loop

done:
	STR R3, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = avail
	BX LR

early_exit:
	STR R2, [R0, #CIRCULAR_AVAIL]	@ circular_list->avail = NULL
	BX LR
