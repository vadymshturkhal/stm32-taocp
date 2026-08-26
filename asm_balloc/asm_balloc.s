.syntax unified
    .thumb
    .cpu cortex-m4

@ Bump Allocator or Arena Allocator
@ Allocates at least 8 bytes

.section .data
.balign 4
heap_head: .word _end              	@ first byte after .bss
heap_end:  .word _stack_limit		@ _stack_limit = _estack - 2048; (in.ld file)

.section .text
	.global heap_head
	.global asm_balloc
	.type asm_balloc, %function
	.thumb_func

@ Input:
@ R0 bytes quantity
asm_balloc:
	CBZ R0, exception

	ADDS R0, R0, #7			@ ADD 7 to the requested size
	BCS exception			@ If overflow

	MOVS R1, #7
	BICS R0, R0, R1			@ Bitwise Clear AND NOT the lower 3 bits

	LDR R1, =heap_head		@ Load address from the heap_head
	LDR R2, [R1]			@ Current free memory address

	ADDS R3, R2, R0
	BCS exception			@ If overflow

	LDR R0, =heap_end
	LDR R0, [R0]
	CMP R3, R0
	BHI exception

	STR R3, [R1]

done:
	MOVS R0, R2
	BX LR

exception:
	MOVS R0, #0
	BX LR
