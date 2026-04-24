.syntax unified
    .thumb
    .cpu cortex-m4
    .global clang_stack_sequential_algorithm_t
	.type clang_stack_sequential_algorithm_t, %function


clang_stack_sequential_algorithm_t:
        push.w  {r4, r5, r6, r7, r8, r9, r10, r11, lr}
        sub     sp, #12
        mov     r8, r0
        movs    r0, #4
        add.w   r4, r0, r8, lsl #2
        lsls    r0, r4, #1
        add.w   r0, r0, r2, lsl #3
        mov     r10, r3
        mov     r11, r2
        mov     r6, r1
        bl      asm_balloc
        cmp     r0, #0
        beq     .LBB0_11
        cmp.w   r8, #0
        add.w   r9, r0, r4
        str     r4, [sp, #8]
        bmi     .LBB0_8
        mvn.w   r2, r8
        lsls    r2, r2, #30
        mov     r2, r8
        beq     .LBB0_5
        lsls.w  r2, r8, #30
        mov.w   r3, #0
        sub.w   r2, r8, #1
        str.w   r3, [r0, r8, lsl #2]
        str.w   r3, [r9, r8, lsl #2]
        beq     .LBB0_5
        str.w   r3, [r0, r2, lsl #2]
        str.w   r3, [r9, r2, lsl #2]
        and     r2, r8, #3
        cmp     r2, #1
        sub.w   r2, r8, #2
        ittt    ne
        strne.w r3, [r0, r2, lsl #2]
        strne.w r3, [r9, r2, lsl #2]
        subne.w r2, r8, #3
.LBB0_5:
        cmp.w   r8, #3
        blo     .LBB0_8
        lsl.w   r3, r8, #2
        adds    r5, r2, #1
        add.w   r2, r0, r2, lsl #2
        movs    r4, #0
.LBB0_7:
        str     r4, [r2]
        adds    r7, r2, r3
        str     r4, [r2, #-4]
        str     r4, [r2, r3]
        str     r4, [r2, #-8]
        str     r4, [r2, #-12]
        subs    r5, #4
        sub.w   r2, r2, #16
        str     r4, [r7, #4]
        str     r4, [r7, #-4]
        str     r4, [r7, #-8]
        bne     .LBB0_7
.LBB0_8:
        subs.w  lr, r11, #1
        bmi.w   .LBB0_19
        mov     r1, r6
        cmp.w   lr, #3
        and     r6, r11, #3
        bhs     .LBB0_12
        movs    r7, #0
        b       .LBB0_16
.LBB0_11:
        movs    r4, #0
        mov     r0, r4
        add     sp, #12
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
.LBB0_12:
        add.w   r3, r0, r8, lsl #3
        add.w   r5, r3, #36
        add.w   r3, r1, r11, lsl #3
        bic     r12, r11, #3
        subs    r3, #16
        movs    r7, #0
        str     r6, [sp]
        str     r1, [sp, #4]
.LBB0_13:
        ldr     r4, [r3, #12]
        ldr     r6, [r3, #8]
        ldr.w   r2, [r0, r4, lsl #2]
        ldr.w   r1, [r9, r6, lsl #2]
        adds    r2, #1
        str.w   r2, [r0, r4, lsl #2]
        mov     r2, r5
        str     r1, [r5, #-24]
        ldr     r1, [r3, #4]
        str     r4, [r2, #-28]!
        str.w   r2, [r9, r6, lsl #2]
        ldr.w   r2, [r0, r1, lsl #2]
        ldr     r4, [r3]
        adds    r2, #1
        str.w   r2, [r0, r1, lsl #2]
        mov     r2, r5
        ldr.w   r6, [r9, r4, lsl #2]
        str     r1, [r2, #-20]!
        ldr     r1, [r3, #-4]
        str     r6, [r5, #-16]
        str.w   r2, [r9, r4, lsl #2]
        ldr.w   r2, [r0, r1, lsl #2]
        ldr     r4, [r3, #-8]
        adds    r2, #1
        str.w   r2, [r0, r1, lsl #2]
        mov     r2, r5
        ldr.w   r6, [r9, r4, lsl #2]
        str     r1, [r2, #-12]!
        ldr     r1, [r3, #-12]
        str     r6, [r5, #-8]
        str.w   r2, [r9, r4, lsl #2]
        ldr.w   r2, [r0, r1, lsl #2]
        ldr     r4, [r3, #-16]
        adds    r2, #1
        str.w   r2, [r0, r1, lsl #2]
        mov     r2, r5
        str     r1, [r2, #-4]!
        ldr.w   r1, [r9, r4, lsl #2]
        adds    r7, #4
        str     r1, [r5], #32
        cmp     r12, r7
        sub.w   r3, r3, #32
        str.w   r2, [r9, r4, lsl #2]
        bne     .LBB0_13
        ldr     r6, [sp]
        ldr     r1, [sp, #4]
        cbz     r6, .LBB0_19
        sub.w   lr, lr, r7
.LBB0_16:
        add.w   r11, r1, lr, lsl #3
        ldr.w   r3, [r11, #4]
        ldr.w   r5, [r1, lr, lsl #3]
        ldr.w   r4, [r0, r3, lsl #2]
        ldr     r1, [sp, #8]
        adds    r4, #1
        add.w   r2, r9, r1
        str.w   r4, [r0, r3, lsl #2]
        ldr.w   r4, [r9, r5, lsl #2]
        str.w   r3, [r2, r7, lsl #3]
        add.w   r3, r2, r7, lsl #3
        cmp     r6, #1
        str     r4, [r3, #4]
        str.w   r3, [r9, r5, lsl #2]
        beq     .LBB0_19
        ldrd    r2, r7, [r11, #-8]
        cmp     r6, #2
        ldr.w   r5, [r0, r7, lsl #2]
        ldr.w   r4, [r9, r2, lsl #2]
        add.w   r5, r5, #1
        str.w   r5, [r0, r7, lsl #2]
        mov     r5, r3
        str     r7, [r5, #8]!
        str     r4, [r5, #4]
        str.w   r5, [r9, r2, lsl #2]
        beq     .LBB0_19
        ldrd    r2, r7, [r11, #-16]
        ldr.w   r5, [r0, r7, lsl #2]
        adds    r5, #1
        str.w   r5, [r0, r7, lsl #2]
        ldr.w   r5, [r9, r2, lsl #2]
        str     r7, [r3, #16]!
        str     r5, [r3, #4]
        str.w   r3, [r9, r2, lsl #2]
.LBB0_19:
        cmp.w   r8, #0
        beq     .LBB0_22
        cmp.w   r8, #4
        and     r12, r8, #3
        bhs     .LBB0_23
        movs    r2, #0
        mov     r5, r8
        b       .LBB0_26
.LBB0_22:
        movs    r4, #1
        bl      asm_balloc_free
        mov     r0, r4
        add     sp, #12
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
.LBB0_23:
        add.w   r2, r0, r8, lsl #2
        bic     r7, r8, #3
        subs    r3, r2, #4
        movs    r2, #0
        mov     r5, r8
.LBB0_24:
        ldr     r1, [r3, #4]
        cmp     r1, #0
        itt     eq
        streq   r2, [r3, #4]
        moveq   r2, r5
        ldr     r1, [r3]
        cmp     r1, #0
        itt     eq
        streq   r2, [r3]
        subeq   r2, r5, #1
        ldr     r1, [r3, #-4]
        cmp     r1, #0
        itt     eq
        streq   r2, [r3, #-4]
        subeq   r2, r5, #2
        ldr     r1, [r3, #-8]
        cmp     r1, #0
        itt     eq
        streq   r2, [r3, #-8]
        subeq   r2, r5, #3
        subs    r5, #4
        subs    r7, #4
        sub.w   r3, r3, #16
        bne     .LBB0_24
        cmp.w   r12, #0
        beq     .LBB0_29
.LBB0_26:
        ldr.w   r1, [r0, r5, lsl #2]
        cmp     r1, #0
        itt     eq
        streq.w r2, [r0, r5, lsl #2]
        moveq   r2, r5
        cmp.w   r12, #1
        beq     .LBB0_29
        subs    r3, r5, #1
        ldr.w   r1, [r0, r3, lsl #2]
        cmp     r1, #0
        ite     eq
        streq.w r2, [r0, r3, lsl #2]
        movne   r3, r2
        cmp.w   r12, #2
        bne     .LBB0_38
        mov     r2, r3
.LBB0_29:
        cbz     r2, .LBB0_39
.LBB0_30:
        movs    r3, #0
        b       .LBB0_32
.LBB0_31:
        adds    r3, #1
        sub.w   r8, r8, #1
        cbz     r2, .LBB0_37
.LBB0_32:
        str.w   r2, [r10, r3, lsl #2]
        ldr.w   r7, [r9, r2, lsl #2]
        ldr.w   r2, [r0, r2, lsl #2]
        b       .LBB0_34
.LBB0_33:
        ldr     r7, [r7, #4]
.LBB0_34:
        cmp     r7, #0
        beq     .LBB0_31
        ldr     r1, [r7]
        ldr.w   r6, [r0, r1, lsl #2]
        subs    r6, #1
        str.w   r6, [r0, r1, lsl #2]
        bne     .LBB0_33
        ldr     r1, [r7]
        str.w   r2, [r0, r1, lsl #2]
        ldr     r2, [r7]
        b       .LBB0_33
.LBB0_37:
        clz     r1, r8
        lsrs    r4, r1, #5
        bl      asm_balloc_free
        mov     r0, r4
        add     sp, #12
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
.LBB0_38:
        subs    r2, r5, #2
        ldr.w   r1, [r0, r2, lsl #2]
        cmp     r1, #0
        ite     eq
        streq.w r3, [r0, r2, lsl #2]
        movne   r2, r3
        cmp     r2, #0
        bne     .LBB0_30
.LBB0_39:
        movs    r4, #0
        bl      asm_balloc_free
        mov     r0, r4
        add     sp, #12
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
