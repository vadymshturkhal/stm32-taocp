.syntax unified
    .thumb
    .cpu cortex-m4
    .global clang_asm_circular_list_init
	.type clang_asm_circular_list_init, %function

clang_asm_circular_list_init:
        movs    r2, #0
        str     r2, [r0, #12]
        subs    r3, r1, #1
        add.w   r2, r0, #8
        beq     .LBB0_13
        push    {r4, r5, r7, lr}
        ands    r4, r3, #3
        sub.w   r12, r1, #2
        beq     .LBB0_4
        cmp     r4, #1
        add.w   r5, r0, #16
        str     r2, [r0, #20]
        bne     .LBB0_5
        mov     lr, r5
        mov     r3, r12
        b       .LBB0_8
.LBB0_4:
        mov     lr, r2
        b       .LBB0_9
.LBB0_5:
        cmp     r4, #2
        add.w   r2, r0, #24
        str     r5, [r0, #28]
        bne     .LBB0_7
        subs    r3, r1, #3
        mov     lr, r2
        mov     r0, r5
        b       .LBB0_9
.LBB0_7:
        add.w   lr, r0, #32
        subs    r3, r1, #4
        str     r2, [r0, #36]
.LBB0_8:
        mov     r0, r2
.LBB0_9:
        cmp.w   r12, #3
        mov     r2, lr
        blo     .LBB0_12
        mov     r2, lr
.LBB0_11:
        str     r2, [r0, #20]
        add.w   r0, r2, #8
        str     r0, [r2, #20]
        add.w   r0, r2, #16
        str     r0, [r2, #28]
        add.w   r0, r2, #24
        str     r0, [r2, #36]
        subs    r3, #4
        add.w   r2, r2, #32
        bne     .LBB0_11
.LBB0_12:
        pop.w   {r4, r5, r7, lr}
.LBB0_13:
        mov     r0, r2
        bx      lr

c_create_circular_list:
        movs    r2, #0
        str     r2, [r0]
        str     r2, [r0, #12]
        subs    r2, r1, #1
        add.w   r3, r0, #8
        beq     .LBB1_13
        push    {r4, lr}
        ands    r4, r2, #3
        sub.w   lr, r1, #2
        beq     .LBB1_4
        cmp     r4, #1
        add.w   r12, r0, #16
        str     r3, [r0, #20]
        bne     .LBB1_5
        mov     r4, r12
        mov     r2, lr
        b       .LBB1_8
.LBB1_4:
        mov     r4, r3
        mov     r12, r0
        b       .LBB1_9
.LBB1_5:
        cmp     r4, #2
        add.w   r3, r0, #24
        str.w   r12, [r0, #28]
        bne     .LBB1_7
        subs    r2, r1, #3
        mov     r4, r3
        b       .LBB1_9
.LBB1_7:
        add.w   r4, r0, #32
        subs    r2, r1, #4
        str     r3, [r0, #36]
.LBB1_8:
        mov     r12, r3
.LBB1_9:
        cmp.w   lr, #3
        mov     r3, r4
        blo     .LBB1_12
        mov     r3, r4
.LBB1_11:
        add.w   r1, r3, #8
        str.w   r3, [r12, #20]
        str     r1, [r3, #20]
        add.w   r1, r3, #16
        add.w   r12, r3, #24
        str     r1, [r3, #28]
        str.w   r12, [r3, #36]
        subs    r2, #4
        add.w   r3, r3, #32
        bne     .LBB1_11
.LBB1_12:
        pop.w   {r4, lr}
.LBB1_13:
        str     r3, [r0, #4]
        bx      lr
