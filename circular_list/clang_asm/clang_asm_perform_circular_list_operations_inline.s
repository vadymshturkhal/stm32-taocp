.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_perform_circular_list_operations_inline
	.type clang_asm_perform_circular_list_operations_inline, %function

clang_asm_perform_circular_list_operations_inline:
	.fnstart
	.save	{r4, r5, r6, r7, lr}
	push	{r4, r5, r6, r7, lr}
	.setfp	r7, sp, #12
	add	r7, sp, #12
	.save	{r11}
	str	r11, [sp, #-4]!
	cbz	r0, .LBB0_11
	mov	r5, r0
	movs	r0, #8
	add.w	r0, r0, r5, lsl #3
	bl	asm_balloc
	cbz	r0, .LBB0_11
	mov	r1, r5
	mov	r6, r0
	bl	c_create_circular_list
	mov	r1, r5
	.p2align	2
.LBB0_3:
	ldr	r2, [r0, #4]
	cbz	r2, .LBB0_15
	ldr	r3, [r0]
	ldr	r4, [r2, #4]
	str	r1, [r2]
	cmp	r3, #0
	str	r4, [r0, #4]
	itee	ne
	ldrne	r4, [r3, #4]!
	moveq	r4, r2
	moveq	r3, r0
	subs	r1, #1
	str	r4, [r2, #4]
	str	r2, [r3]
	bne	.LBB0_3
	ldr	r3, [r0]
	mov	r2, r5
	b	.LBB0_8
	.p2align	2
.LBB0_6:
	ldr	r4, [r1, #4]
	str	r4, [r3, #4]
.LBB0_7:
	ldr	r4, [r0, #4]
	subs	r2, #1
	str	r4, [r1, #4]
	str	r1, [r0, #4]
	beq	.LBB0_12
.LBB0_8:
	cbz	r3, .LBB0_15
	ldr	r1, [r3, #4]
	cmp	r3, r1
	bne	.LBB0_6
	movs	r3, #0
	str	r3, [r0]
	b	.LBB0_7
	.p2align	2
.LBB0_11:
	movs	r5, #0
	mov	r0, r5
	ldr	r11, [sp], #4
	pop	{r4, r5, r6, r7, pc}
	.p2align	2
.LBB0_12:
	cbz	r1, .LBB0_15
	.p2align	2
.LBB0_13:
	ldr	r3, [r1, #4]
	ldr	r2, [r0]
	str	r5, [r1]
	str	r3, [r0, #4]
	cmp	r2, #0
	itee	ne
	ldrne	r3, [r2, #4]!
	moveq	r3, r1
	moveq	r2, r0
	str	r3, [r1, #4]
	str	r1, [r2]
	ldr	r1, [r0]
	subs	r5, #1
	ldr	r1, [r1, #4]
	str	r1, [r0]
	beq	.LBB0_17
	ldr	r1, [r0, #4]
	cmp	r1, #0
	bne	.LBB0_13
.LBB0_15:
	movs	r5, #0
.LBB0_16:
	mov	r0, r6
	bl	asm_balloc_free
	mov	r0, r5
	ldr	r11, [sp], #4
	pop	{r4, r5, r6, r7, pc}
	.p2align	2
.LBB0_17:
	bl	circular_list_clear
	movs	r5, #1
	b	.LBB0_16
