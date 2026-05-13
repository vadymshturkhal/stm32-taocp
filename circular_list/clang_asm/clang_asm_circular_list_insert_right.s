.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_circular_list_insert_right
	.type clang_asm_circular_list_insert_right, %function


clang_asm_circular_list_insert_right:
	.fnstart
	mov	r2, r0
	ldr	r0, [r0, #4]
	cbz	r0, .LBB3_2
	ldr	r3, [r0, #4]
	str	r1, [r0]
	ldr	r1, [r2]
	str	r3, [r2, #4]
	cmp	r1, #0
	itee	ne
	ldrne	r3, [r1, #4]!
	moveq	r3, r0
	moveq	r1, r2
	str	r3, [r0, #4]
	str	r0, [r1]
	ldr	r1, [r2]
	ldr	r1, [r1, #4]
	str	r1, [r2]
.LBB3_2:
	cmp	r0, #0
	it	ne
	movne	r0, #1
	bx	lr
