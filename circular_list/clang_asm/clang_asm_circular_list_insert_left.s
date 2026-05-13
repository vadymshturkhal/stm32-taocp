.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_circular_list_insert_left
	.type clang_asm_circular_list_insert_left, %function


clang_asm_circular_list_insert_left:
	.fnstart
	mov	r2, r0
	ldr	r0, [r0, #4]
	cbz	r0, .LBB2_2
	str	r1, [r0]
	ldr	r1, [r0, #4]
	ldr	r3, [r2]
	str	r1, [r2, #4]
	cmp	r3, #0
	itte	ne
	ldrne	r1, [r3, #4]!
	movne	r2, r3
	moveq	r1, r0
	str	r1, [r0, #4]
	str	r0, [r2]
.LBB2_2:
	cmp	r0, #0
	it	ne
	movne	r0, #1
	bx	lr
