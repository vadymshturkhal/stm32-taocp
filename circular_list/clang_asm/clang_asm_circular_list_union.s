.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_circular_list_union
	.type clang_asm_circular_list_union, %function


clang_asm_circular_list_union:
	.fnstart
	.save	{r7, lr}
	push	{r7, lr}
	.setfp	r7, sp
	mov	r7, sp
	ldr	r2, [r1]
	cmp	r2, #0
	it	eq
	popeq	{r7, pc}
.LBB6_1:
	ldr	r3, [r0]
	cbz	r3, .LBB6_3
	ldr.w	r12, [r2, #4]
	ldr.w	lr, [r3, #4]
	str.w	r12, [r3, #4]
	str.w	lr, [r2, #4]
.LBB6_3:
	str	r2, [r0]
	movs	r0, #0
	str	r0, [r1]
	pop	{r7, pc}
