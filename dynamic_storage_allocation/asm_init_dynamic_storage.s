.syntax unified
    .thumb
    .cpu cortex-m4

@ Dynamic Storage Allocator
@ Allocate First and Last AVAIL blocks

.section .bss				@ Block Started by Symbol (Uninitialized Memory)
.balign 16
dynamic_storage_base:
	.space 21248			@ 0x5300
dynamic_storage_end:

.section .data				@ Initialized Memory
.balign 4
dynamic_heap_head:
	.word dynamic_storage_base	@ Points to the start of our new memory pool

.section .text
	.global dynamic_heap_head
	.global asm_init_dynamic_storage

.equ P_SIZE, 0
.equ P_LINK_NEXT, 4
.equ P_LINK_PREV, 8
.equ FIELDS_SIZE, 16	@ Align to 8
.equ HEAP_SIZE, 21248
.equ HEAP_AVAIL, HEAP_SIZE - FIELDS_SIZE
@ .equ HEAP_AVAIL, HEAP_SIZE - 2 * FIELDS_SIZE

@ Return heap head (pointer to the first block)
asm_init_dynamic_storage:
	LDR R0, =dynamic_heap_head	@ Load address of the heap_head pointer
	LDR R1, [R0]				@ R1 = dynamic storage base

	LDR R2, =HEAP_AVAIL			@ R2 = HEAP_AVAIL
	STR R2, [R1, #P_SIZE]		@ block->size = AVAIL

	MOVS R3, #0
	STR R3, [R1, #P_LINK_NEXT]	@ first_block->next = NULL
	STR R3, [R1, #P_LINK_PREV]	@ first_block->prev = NULL

	@ last block
	@ ADDS R3, R1, #FIELDS_SIZE
	@ ADDS R3, R3, R2         	@ R3 now points to the last 8 bytes of the arena
	@ STR R3, [R1, #P_LINK_NEXT]	@ Store address of the last AVAIL block to the first block link

	@ MOVS R2, #0
	@ STR R2, [R1, #P_LINK_PREV]	@ first_block->prev = NULL

	@ STR R2, [R3, #P_SIZE]		@ last_block->size = 0
	@ STR R2, [R3, #P_LINK_NEXT]	@ last_block->link = NULL
	@ STR R1, [R3, #P_LINK_PREV]	@ last_block->prev = first_block location

done:
	MOVS R0, R1
	BX LR
