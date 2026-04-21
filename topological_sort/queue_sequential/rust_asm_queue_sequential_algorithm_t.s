.syntax unified
    .thumb
    .cpu cortex-m4
    .global rust_asm_algorithm_t
    .type rust_asm_algorithm_t, %function


__rustc_rust_begin_unwind:
        push    {r7, lr}
        mov     r7, sp
.LBB0_1:
        b       .LBB0_1

rust_asm_algorithm_t:
        push    {r4, r5, r6, r7, lr}
        add     r7, sp, #12
        push.w  {r8, r9, r10, r11}
        sub     sp, #20
        mov     r8, r0
        movs    r0, #4
        add.w   r4, r0, r8, lsl #2
        mov     r0, r4
        mov     r5, r3
        mov     r10, r2
        mov     r6, r1
        bl      asm_balloc
        cmp     r0, #0
        beq     .LBB1_15
        mov     r11, r0
        mov     r0, r4
        bl      asm_balloc
        cmp     r0, #0
        beq     .LBB1_15
        strd    r5, r6, [sp, #12]
        lsl.w   r5, r10, #3
        mov     r9, r0
        mov     r0, r5
        bl      asm_balloc
        cmp     r0, #0
        mov.w   r4, #0
        beq     .LBB1_16
        mov     r1, r0
        mov.w   r2, #-1
        movs    r3, #0
        mov     r0, r11
.LBB1_4:
        cmp     r3, r8
        mov.w   r6, #0
        it      lo
        movlo   r6, #1
        str.w   r4, [r0, r3, lsl #2]
        str.w   r2, [r9, r3, lsl #2]
        bhs     .LBB1_12
        add     r3, r6
        cmp     r3, r8
        bhi     .LBB1_12
        mov.w   r6, #0
        it      lo
        movlo   r6, #1
        str.w   r4, [r0, r3, lsl #2]
        str.w   r2, [r9, r3, lsl #2]
        bhs     .LBB1_12
        add     r3, r6
        cmp     r3, r8
        bhi     .LBB1_12
        mov.w   r6, #0
        it      lo
        movlo   r6, #1
        str.w   r4, [r0, r3, lsl #2]
        str.w   r2, [r9, r3, lsl #2]
        bhs     .LBB1_12
        add     r3, r6
        cmp     r3, r8
        bhi     .LBB1_12
        mov.w   r6, #0
        it      lo
        movlo   r6, #1
        str.w   r4, [r0, r3, lsl #2]
        str.w   r2, [r9, r3, lsl #2]
        bhs     .LBB1_12
        add     r3, r6
        cmp     r3, r8
        bls     .LBB1_4
.LBB1_12:
        cmp.w   r10, #0
        beq.w   .LBB1_23
        ldr     r2, [sp, #16]
        sub.w   r3, r5, #8
        adds    r4, r2, r5
        movs    r2, #1
        add.w   r2, r2, r3, lsr #3
        cmp     r3, #24
        and     r5, r2, #3
        bhs     .LBB1_17
        mov.w   lr, #0
        cmp     r5, #0
        bne     .LBB1_20
        b       .LBB1_23
.LBB1_15:
        movs    r4, #0
.LBB1_16:
        mov     r0, r4
        add     sp, #20
        pop.w   {r8, r9, r10, r11}
        pop     {r4, r5, r6, r7, pc}
.LBB1_17:
        bic     r2, r2, #3
        str     r2, [sp, #16]
        add.w   r11, r1, #16
        str     r4, [sp, #8]
        sub.w   r6, r4, #32
        movs    r2, #0
        movs    r4, #0
        str     r5, [sp, #4]
.LBB1_18:
        ldr     r3, [r6, #28]
        ldr.w   lr, [r6, #24]
        ldr.w   r5, [r0, r3, lsl #2]
        add.w   r12, r1, r2
        adds    r5, #1
        str.w   r5, [r0, r3, lsl #2]
        ldr.w   r5, [r9, lr, lsl #2]
        str     r3, [r1, r2]
        str.w   r5, [r12, #4]
        str.w   r4, [r9, lr, lsl #2]
        ldr     r3, [r6, #20]
        ldr.w   r10, [r6, #16]
        ldr.w   lr, [r0, r3, lsl #2]
        add.w   r5, lr, #1
        str.w   r5, [r0, r3, lsl #2]
        ldr.w   r5, [r9, r10, lsl #2]
        str.w   r3, [r12, #8]
        add.w   r3, r11, r2
        str     r5, [r3, #-4]
        adds    r3, r4, #1
        str.w   r3, [r9, r10, lsl #2]
        ldr     r3, [r6, #12]
        ldr.w   r10, [r6, #8]
        ldr.w   lr, [r0, r3, lsl #2]
        add.w   r5, lr, #1
        str.w   r5, [r0, r3, lsl #2]
        ldr.w   r5, [r9, r10, lsl #2]
        str.w   r3, [r11, r2]
        adds    r3, r4, #2
        str.w   r5, [r12, #20]
        str.w   r3, [r9, r10, lsl #2]
        ldr     r5, [r6, #4]
        ldr     r10, [r6], #-32
        ldr.w   lr, [r0, r5, lsl #2]
        adds    r2, #32
        add.w   r3, lr, #1
        str.w   r3, [r0, r5, lsl #2]
        ldr.w   r3, [r9, r10, lsl #2]
        add.w   lr, r4, #4
        strd    r5, r3, [r12, #24]
        adds    r3, r4, #3
        str.w   r3, [r9, r10, lsl #2]
        ldr     r3, [sp, #16]
        mov     r4, lr
        cmp     lr, r3
        bne     .LBB1_18
        ldr     r4, [sp, #8]
        ldr     r5, [sp, #4]
        subs    r4, r4, r2
        cbz     r5, .LBB1_23
.LBB1_20:
        ldrd    r2, r3, [r4, #-8]
        cmp     r5, #1
        ldr.w   r6, [r0, r3, lsl #2]
        add.w   r6, r6, #1
        str.w   r6, [r0, r3, lsl #2]
        ldr.w   r12, [r9, r2, lsl #2]
        add.w   r6, r1, lr, lsl #3
        str.w   r3, [r1, lr, lsl #3]
        str.w   r12, [r6, #4]
        str.w   lr, [r9, r2, lsl #2]
        beq     .LBB1_23
        ldrd    r10, r3, [r4, #-16]
        add.w   r2, lr, #1
        ldr.w   r6, [r0, r3, lsl #2]
        cmp     r5, #2
        add.w   r6, r6, #1
        str.w   r6, [r0, r3, lsl #2]
        ldr.w   r12, [r9, r10, lsl #2]
        add.w   r6, r1, r2, lsl #3
        str.w   r3, [r1, r2, lsl #3]
        str.w   r12, [r6, #4]
        str.w   r2, [r9, r10, lsl #2]
        beq     .LBB1_23
        ldrd    r2, r3, [r4, #-24]
        add.w   r5, lr, #2
        ldr.w   r6, [r0, r3, lsl #2]
        add.w   r4, r1, r5, lsl #3
        adds    r6, #1
        str.w   r6, [r0, r3, lsl #2]
        ldr.w   r6, [r9, r2, lsl #2]
        str.w   r3, [r1, r5, lsl #3]
        str     r6, [r4, #4]
        str.w   r5, [r9, r2, lsl #2]
.LBB1_23:
        movs    r2, #0
        cmp.w   r8, #0
        str     r2, [r0]
        beq     .LBB1_33
        ldr.w   lr, [sp, #12]
        movs    r5, #0
        mov     r3, r8
.LBB1_25:
        ldr.w   r2, [r0, r3, lsl #2]
        subs    r6, r3, #1
        it      eq
        moveq   r6, r3
        cmp     r2, #0
        itt     eq
        streq.w r3, [r0, r5, lsl #2]
        moveq   r5, r3
        cmp     r3, #1
        beq     .LBB1_27
        cmp     r6, #0
        mov     r3, r6
        bne     .LBB1_25
.LBB1_27:
        ldr     r3, [r0]
        cbz     r3, .LBB1_34
        mov.w   r12, #0
.LBB1_29:
        str.w   r3, [lr, r12, lsl #2]
        ldr.w   r4, [r9, r3, lsl #2]
        adds    r2, r4, #1
        beq     .LBB1_31
.LBB1_30:
        ldr.w   r2, [r1, r4, lsl #3]
        ldr.w   r6, [r0, r2, lsl #2]
        subs    r6, #1
        str.w   r6, [r0, r2, lsl #2]
        itt     eq
        streq.w r2, [r0, r5, lsl #2]
        moveq   r5, r2
        add.w   r2, r1, r4, lsl #3
        ldr     r4, [r2, #4]
        adds    r2, r4, #1
        bne     .LBB1_30
.LBB1_31:
        sub.w   r8, r8, #1
        uxtb.w  r2, r8
        clz     r4, r2
        lsr.w   r4, r4, #5
        cbz     r2, .LBB1_35
        ldr.w   r3, [r0, r3, lsl #2]
        add.w   r12, r12, #1
        cmp     r3, #0
        bne     .LBB1_29
        b       .LBB1_35
.LBB1_33:
        movs    r4, #1
        b       .LBB1_35
.LBB1_34:
        movs    r4, #0
.LBB1_35:
        bl      asm_balloc_free
        mov     r0, r4
        add     sp, #20
        pop.w   {r8, r9, r10, r11}
        pop     {r4, r5, r6, r7, pc}
