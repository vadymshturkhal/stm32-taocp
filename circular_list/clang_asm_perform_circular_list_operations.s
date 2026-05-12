.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_perform_circular_list_operations
	.type clang_asm_perform_circular_list_operations, %function

clang_asm_perform_circular_list_operations:
	.fnstart
	.save	{r4, r5, r6, r7, lr}
	push	{r4, r5, r6, r7, lr}
	.setfp	r7, sp, #12
	add	r7, sp, #12
	.save	{r8, r9, r11}
	push.w	{r8, r9, r11}
	.pad	#8
	sub	sp, #8
	cbz	r0, .LBB0_12
	mov	r4, r0
	movs	r0, #8
	add.w	r0, r0, r4, lsl #3
	bl	asm_balloc
	cbz	r0, .LBB0_12
	mov	r1, r4
	mov	r8, r0
	bl	c_create_circular_list
	mov	r6, r0
	mov	r5, r4
	.p2align	2
.LBB0_3:
	mov	r0, r6
	mov	r1, r5
	bl	circular_list_insert_left
	cbz	r0, .LBB0_11
	subs	r5, #1
	bne	.LBB0_3
	movs	r0, #1
	mvn	r5, #127
	sub.w	r9, r7, #25
	strb	r0, [r7, #-25]
	.p2align	2
.LBB0_6:
	mov	r0, r9
	mov	r1, r6
	bl	circular_list_pop
	ldrb	r0, [r7, #-25]
	cbz	r0, .LBB0_10
	adds	r5, #1
	blo	.LBB0_6
	b	.LBB0_9
	.p2align	2
.LBB0_8:
	subs	r4, #1
	beq	.LBB0_13
.LBB0_9:
	mov	r0, r6
	mov	r1, r4
	bl	circular_list_insert_right
	cmp	r0, #0
	bne	.LBB0_8
.LBB0_10:
	movs	r5, #0
	b	.LBB0_14
	.p2align	2
.LBB0_11:
	mov	r0, r8
	bl	asm_balloc_free
.LBB0_12:
	movs	r5, #0
	mov	r0, r5
	add	sp, #8
	pop.w	{r8, r9, r11}
	pop	{r4, r5, r6, r7, pc}
	.p2align	2
.LBB0_13:
	mov	r0, r6
	bl	circular_list_clear
	movs	r5, #1
.LBB0_14:
	mov	r0, r8
	bl	asm_balloc_free
	mov	r0, r5
	add	sp, #8
	pop.w	{r8, r9, r11}
	pop	{r4, r5, r6, r7, pc}

