.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_create_circular_list_lipski_mod4
	.type asm_create_circular_list_lipski_mod4, %function

@ Used Lipski Trick (MVE mod 4) with hoisting, Duff's Device
@ and Instruction Scheduling for handling Address Generation Interlock (AGI)
@ ~3.2 cycles per node, 100 bytes

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

asm_create_circular_list_lipski_mod4:
	PUSH {R4-R5, LR}
	MOVS R2, #0
	STR R2, [R0, #CIRCULAR_PTR]	@ circular_list->ptr = NULL
	CBZ R1, early_exit

.balign 4
init_storage_pool:
	ADDS R3, R0, #CIRCULAR_SIZE	@ (CircularNode*)(circular_list + 1)
	STR R2, [R3, #NODE_LINK]	@ avail->link = NULL

	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done				@ if there is only 1 node total

	@ Check for remainder 1 (Bit 0)
	TST R1, #1
	BEQ check_mod_2				@ If even, skip to the 2-peel check

peel_1:
	ADDS R2, R3, #NODE_SIZE		@ Calculate next node
	STR R3, [R2, #NODE_LINK]	@ Link to previous
	MOVS R3, R2					@ Update avail
	SUBS R1, R1, #1				@ max_nodes--
	CBZ R1, done				@ Exit if we perfectly finished the list

check_mod_2:
	@ Check for remainder 2 (Bit 1)
	TST R1, #2
	BEQ hoist_tmp				@ If perfectly divisible by 4, jump to hot loop

@ Peel 2 nodes
peel_2:
	ADDS R2, R3, #NODE_SIZE		@ Node A
	STR R3, [R2, #NODE_LINK]
	ADDS R3, R2, #NODE_SIZE		@ Node B
	STR R2, [R3, #NODE_LINK]

	SUBS R1, R1, #2				@ max_nodes -= 2
	CBZ R1, done				@ Exit if we perfectly finished the list

hoist_tmp:
	ADDS R2, R3, #NODE_SIZE		@ Prime the pump for the Mod-4 hot loop

.balign 4
linking_loop:
	@ Node 1
	STR R3, [R2, #NODE_LINK]	@ tmp->link = avail
	ADDS R5, R2, #NODE_SIZE		@ tmp = avail+1
	ADDS R4, R5, #NODE_SIZE		@ tmp = avail+1

	@ Node 2
	STR R2, [R5, #NODE_LINK]	@ tmp->link = avail

	@ Node 3
	STR R5, [R4, #NODE_LINK]	@ tmp->link = avail
	ADDS R3, R4, #NODE_SIZE		@ tmp = avail+1
	ADDS R2, R3, #NODE_SIZE		@ tmp = avail+1

	@ Node 4
	STR R4, [R3, #NODE_LINK]	@ tmp->link = avail

	SUBS R1, R1, #4				@ max_nodes -= 4 (Valid 16-bit instruction!)
	BNE linking_loop

done:
	@ R3 is Avail in all cases: 1 node, peel_1, peel_2 and hot loop
	STR R3, [R0, #CIRCULAR_AVAIL]
	POP {R4-R5, PC}

early_exit:
	STR R2, [R0, #CIRCULAR_AVAIL]
	POP {R4-R5, PC}
