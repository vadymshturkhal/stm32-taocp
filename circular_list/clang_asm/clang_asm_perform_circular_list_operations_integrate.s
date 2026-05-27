.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_perform_circular_list_operations_integrate
	.type clang_asm_perform_circular_list_operations_integrate, %function

clang_asm_perform_circular_list_operations_integrate:
	.fnstart
	.save	{r4, r5, r6, r7, lr}
	push	{r4, r5, r6, r7, lr}
	.setfp	r7, sp, #12
	add	r7, sp, #12
	.save	{r8}
	str	r8, [sp, #-4]!
	cbz	r0, .LBB0_12
	mov	r6, r0
	movs	r0, #8
	add.w	r0, r0, r6, lsl #3
	bl	asm_balloc
	cbz	r0, .LBB0_12
	mov	r1, r6
	mov	r8, r0
	bl	c_create_circular_list
	ldr.w	lr, [r0, #4]
	subs	r1, r6, #1
	cmn.w	r1, #3
	mov	r12, lr
	mov	r3, lr
	bhi	.LBB0_14
	movs	r1, #1
	mov	r3, lr
.LBB0_4:
	cmp	r3, #0
	beq.w	.LBB0_41
	ldr	r2, [r3, #4]
	cmp	r1, r6
	str	r1, [r3]
	beq	.LBB0_13
	cmp	r2, #0
	beq	.LBB0_41
	adds	r1, #1
	ldr	r3, [r2, #4]
	cmp	r1, r6
	str	r1, [r2]
	beq	.LBB0_16
	cmp	r3, #0
	beq	.LBB0_41
	adds	r1, #1
	ldr	r2, [r3, #4]
	cmp	r1, r6
	str	r1, [r3]
	beq	.LBB0_13
	cmp	r2, #0
	beq	.LBB0_41
	adds	r5, r1, #1
	ldr	r3, [r2, #4]
	adds	r1, #2
	cmp	r5, r6
	mov	r12, r2
	str	r5, [r2]
	bne	.LBB0_4
	b	.LBB0_14
	.p2align	2
.LBB0_12:
	movs	r5, #0
	mov	r0, r5
	ldr	r8, [sp], #4
	pop	{r4, r5, r6, r7, pc}
	.p2align	2
.LBB0_13:
	mov	r12, r3
	mov	r3, r2
.LBB0_14:
	ldr	r1, [r0]
	cbz	r1, .LBB0_17
.LBB0_15:
	ldr	r2, [r1, #4]
	str	r3, [r0, #4]
	str.w	r2, [r12, #4]
	str.w	lr, [r1, #4]
	mov	r12, r1
	b	.LBB0_18
	.p2align	2
.LBB0_16:
	mov	r12, r2
	ldr	r1, [r0]
	cmp	r1, #0
	bne	.LBB0_15
.LBB0_17:
	str.w	lr, [r12, #4]
	strd	r12, r3, [r0]
.LBB0_18:
	mov	r5, r6
	mov	r2, lr
.LBB0_19:
	ldr	r1, [r2, #4]
	cmp	r1, lr
	beq	.LBB0_29
	cmp	r5, #1
	beq	.LBB0_27
	ldr	r2, [r1, #4]
	cmp	r2, lr
	beq	.LBB0_28
	cmp	r5, #2
	beq	.LBB0_30
	ldr	r1, [r2, #4]
	cmp	r1, lr
	beq	.LBB0_29
	cmp	r5, #3
	beq	.LBB0_27
	ldr	r2, [r1, #4]
	cmp	r2, lr
	beq	.LBB0_28
	subs	r5, #4
	mov	r4, r1
	bne	.LBB0_19
	b	.LBB0_31
	.p2align	2
.LBB0_27:
	mov	r4, r2
	mov	r2, r1
	b	.LBB0_31
	.p2align	2
.LBB0_28:
	mov	r2, r1
.LBB0_29:
	mov.w	r12, #0
	str.w	r12, [r0]
	cmp.w	lr, #0
	str	r3, [r2, #4]
	str.w	lr, [r0, #4]
	bne	.LBB0_32
	b	.LBB0_41
	.p2align	2
.LBB0_30:
	mov	r4, r1
.LBB0_31:
	str.w	r2, [r12, #4]
	mov	r2, r4
	cmp.w	lr, #0
	str	r3, [r2, #4]
	str.w	lr, [r0, #4]
	beq	.LBB0_41
.LBB0_32:
	mov	r3, lr
.LBB0_33:
	cbz	r3, .LBB0_41
	ldr	r1, [r3, #4]
	subs	r4, r6, #1
	str	r6, [r3]
	beq	.LBB0_42
	cbz	r1, .LBB0_41
	ldr	r2, [r1, #4]
	subs	r3, r4, #1
	str	r4, [r1]
	beq	.LBB0_43
	cbz	r2, .LBB0_41
	ldr	r1, [r2, #4]
	subs	r4, r3, #1
	str	r3, [r2]
	beq	.LBB0_44
	cbz	r1, .LBB0_41
	ldr	r3, [r1, #4]
	subs	r6, r4, #1
	mov	r5, r3
	str	r4, [r1]
	bne	.LBB0_33
	b	.LBB0_45
	.p2align	2
.LBB0_41:
	movs	r5, #0
	b	.LBB0_49
	.p2align	2
.LBB0_42:
	mov	r5, r1
	mov	r1, r3
	b	.LBB0_45
	.p2align	2
.LBB0_43:
	mov	r5, r2
	b	.LBB0_45
	.p2align	2
.LBB0_44:
	mov	r5, r1
	mov	r1, r2
.LBB0_45:
	cmp.w	r12, #0
	beq	.LBB0_47
	ldr.w	r2, [r12, #4]
	str	r2, [r1, #4]
	str.w	lr, [r12, #4]
	b	.LBB0_48
	.p2align	2
.LBB0_47:
	str.w	lr, [r1, #4]
.LBB0_48:
	str	r1, [r0]
	str	r5, [r0, #4]
	bl	circular_list_clear
	movs	r5, #1
.LBB0_49:
	mov	r0, r8
	bl	asm_balloc_free
	mov	r0, r5
	ldr	r8, [sp], #4
	pop	{r4, r5, r6, r7, pc}
