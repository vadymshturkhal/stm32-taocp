.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_init_avail_list_t
	.type asm_init_avail_list_t, %function

@ Memory offset
.equ NODE_INFO,		0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8

@ Input
@ R0 void* memory
@ R1 uint32_t size

@ Runtime
@ R0 Avail
@ R1 size
@ R2 0, tmp

@ Return Avail or 0
asm_init_avail_list_t:
	@ 	if (size == 0 || memory == NULL) return NULL;
	CBZ R0, early_exit
	CBZ R1, early_exit

	MOVS R2, #0
	STR R2, [R0, #NODE_LINK]	@ Avail->link = NULL

	SUBS R1, R1, #1				@ size--
	CBZ R1, done				@ if Avail List is len 1

.balign 4
linking_loop:
	ADDS R2, R0, #NODE_SIZE		@ tmp = Avail + 8 bytes
	STR R0, [R2, #NODE_LINK]	@ tmp->link = Avail

	MOVS R0, R2					@ Avail = tmp
	SUBS R1, R1, #1				@ size--
	BNE linking_loop

done:
	BX LR

early_exit:
	MOVS R0, #0					@ Set return value to NULL
	BX LR
