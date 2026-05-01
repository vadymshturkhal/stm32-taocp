.syntax unified
    .thumb
    .cpu cortex-m4
    .global rust_asm_circular_list_init
	.type rust_asm_circular_list_init, %function

rust_asm_circular_list_init:
        movs    r2, #0
        str     r2, [r0]
        str     r2, [r0, #12]
        subs    r2, r1, #1
        add.w   r3, r0, #8
        beq     .LBB0_13
        push    {r4, r6, r7, lr}
        add     r7, sp, #8
        ands    r4, r2, #3
        sub.w   r12, r1, #2
        beq     .LBB0_4
        cmp     r4, #1
        add.w   lr, r0, #16
        str     r3, [r0, #20]
        bne     .LBB0_5
        mov     r1, lr
        mov     r2, r12
        b       .LBB0_8
.LBB0_4:
        mov     r1, r3
        mov     lr, r0
        b       .LBB0_9
.LBB0_5:
        cmp     r4, #2
        add.w   r3, r0, #24
        str.w   lr, [r0, #28]
        bne     .LBB0_7
        subs    r2, r1, #3
        mov     r1, r3
        b       .LBB0_9
.LBB0_7:
        subs    r2, r1, #4
        add.w   r1, r0, #32
        str     r3, [r0, #36]
.LBB0_8:
        mov     lr, r3
.LBB0_9:
        cmp.w   r12, #3
        mov     r3, r1
        blo     .LBB0_12
        mov     r3, r1
.LBB0_11:
        add.w   r1, r3, #8
        str.w   r3, [lr, #20]
        str     r1, [r3, #20]
        add.w   r1, r3, #16
        add.w   lr, r3, #24
        str     r1, [r3, #28]
        str.w   lr, [r3, #36]
        subs    r2, #4
        add.w   r3, r3, #32
        bne     .LBB0_11
.LBB0_12:
        pop.w   {r4, r6, r7, lr}
.LBB0_13:
        str     r3, [r0, #4]
        bx      lr

