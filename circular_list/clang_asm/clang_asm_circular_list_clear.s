.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_circular_list_clear
	.type clang_asm_circular_list_clear, %function


clang_asm_circular_list_clear:
	.fnstart
	ldr	r1, [r0]
	cmp	r1, #0
	it	eq
	bxeq	lr
.LBB5_1:
	ldr	r3, [r0, #4]
	ldr	r2, [r1, #4]
	str	r3, [r1, #4]
	movs	r1, #0
	str	r2, [r0, #4]
	str	r1, [r0]
	bx	lr
