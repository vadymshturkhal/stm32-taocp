.syntax unified
    .thumb
    .cpu cortex-m4

@ Bump Allocator or Arena Allocator
@ Allocates at least 8 bytes

.section .bss				@ Block Started by Symbol (Uninitialized Memory)
.balign 8
custom_heap_base:
	.space 16384			@ 16 * 1024
custom_heap_end:

.section .data				@ Initialized Memory
.balign 4
heap_head:
	.word custom_heap_base	@ Create heap_head read/write variable in RAM with init value of custom_heap_base

.section .text
	.global heap_head
	.global asm_balloc

@ Input:
@ R0 bytes quantity

asm_balloc:
	CBZ R0, exception

	ADDS R0, R0, #7			@ ADD 7 to the requested size
	MOVS R1, #7
	BICS R0, R0, R1			@ Bitwise Clear AND NOT the lower 3 bits

	LDR R1, =heap_head		@ Load address from the heap_head
	LDR R2, [R1]			@ Current free memory address

	ADDS R3, R2, R0

	LDR R0, =custom_heap_end
	CMP R3, R0
	BHI exception

	STR R3, [R1]

done:
	MOVS R0, R2
	BX LR

exception:
	MOVS R0, #0
	BX LR
