.syntax unified
    .thumb
    .cpu cortex-m4
    .global clang_algorithm_t_sequential
	.type clang_algorithm_t_sequential, %function


clang_algorithm_t_sequential:
        push.w  {r4, r5, r6, r7, r8, r9, r10, r11, lr}
        sub     sp, #20
        mov     r8, r0
        movs    r0, #4
        add.w   r5, r0, r8, lsl #2
        mov     r0, r5
        mov     r11, r3
        mov     r10, r2
        mov     r6, r1
        bl      asm_balloc
        cbz     r0, .LBB0_6
        mov     r1, r5
        mov     r4, r0
        bl      __aeabi_memclr4
        mov     r0, r5
        bl      asm_balloc
        cmp     r0, #0
        beq.w   .LBB0_39
        mov     r1, r5
        mov     r9, r0
        bl      __aeabi_memclr4
        cmp.w   r10, #0
        beq.w   .LBB0_14
        lsl.w   r0, r10, #3
        bl      asm_balloc
        cmp     r0, #0
        beq.w   .LBB0_39
        cmp.w   r10, #4
        and     r5, r10, #3
        bhs     .LBB0_7
        movs    r1, #0
        b       .LBB0_11
.LBB0_6:
        movs    r5, #0
        mov     r0, r5
        add     sp, #20
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
.LBB0_7:
        mvn     r1, #24
        str.w   r11, [sp, #16]
        and.w   r11, r1, r10, lsl #3
        add.w   r1, r6, r10, lsl #3
        str     r5, [sp, #8]
        sub.w   r5, r1, #32
        and     r1, r10, #252
        str     r1, [sp, #4]
        movs    r1, #0
        str     r6, [sp, #12]
.LBB0_8:
        ldr     r3, [r5, #28]
        ldr.w   r12, [r5, #24]
        ldr.w   r2, [r4, r3, lsl #2]
        ldr.w   lr, [r9, r12, lsl #2]
        adds    r2, #1
        str.w   r2, [r4, r3, lsl #2]
        str     r3, [r0, r1]
        ldr     r3, [r5, #20]
        adds    r2, r0, r1
        ldr.w   r7, [r4, r3, lsl #2]
        ldr     r6, [r5, #16]
        adds    r7, #1
        str.w   lr, [r2, #4]
        str.w   r2, [r9, r12, lsl #2]
        str.w   r7, [r4, r3, lsl #2]
        mov     r7, r2
        ldr.w   r12, [r9, r6, lsl #2]
        str     r3, [r7, #8]!
        ldr     r3, [r5, #12]
        str.w   r12, [r2, #12]
        str.w   r7, [r9, r6, lsl #2]
        ldr.w   r7, [r4, r3, lsl #2]
        ldr     r6, [r5, #8]
        adds    r7, #1
        str.w   r7, [r4, r3, lsl #2]
        mov     r7, r2
        ldr.w   r12, [r9, r6, lsl #2]
        str     r3, [r7, #16]!
        ldr     r3, [r5, #4]
        str.w   r12, [r2, #20]
        str.w   r7, [r9, r6, lsl #2]
        ldr.w   r7, [r4, r3, lsl #2]
        ldr     r6, [r5], #-32
        add.w   r12, r7, #1
        ldr.w   r7, [r9, r6, lsl #2]
        adds    r1, #32
        str.w   r12, [r4, r3, lsl #2]
        str     r7, [r2, #28]
        str     r3, [r2, #24]!
        cmp     r11, r1
        str.w   r2, [r9, r6, lsl #2]
        bne     .LBB0_8
        ldr     r5, [sp, #8]
        ldrd    r6, r11, [sp, #12]
        cbz     r5, .LBB0_14
        ldr     r1, [sp, #4]
        sub.w   r10, r10, r1
.LBB0_11:
        add.w   r6, r6, r10, lsl #3
        ldrd    r2, r3, [r6, #-8]
        cmp     r5, #1
        ldr.w   r7, [r4, r3, lsl #2]
        add.w   r7, r7, #1
        str.w   r7, [r4, r3, lsl #2]
        str.w   r3, [r0, r1, lsl #3]
        ldr.w   r3, [r9, r2, lsl #2]
        add.w   r0, r0, r1, lsl #3
        str     r3, [r0, #4]
        str.w   r0, [r9, r2, lsl #2]
        beq     .LBB0_14
        ldrd    r2, r3, [r6, #-16]
        cmp     r5, #2
        ldr.w   r7, [r4, r3, lsl #2]
        ldr.w   r1, [r9, r2, lsl #2]
        add.w   r7, r7, #1
        str.w   r7, [r4, r3, lsl #2]
        mov     r7, r0
        str     r3, [r7, #8]!
        str     r1, [r7, #4]
        str.w   r7, [r9, r2, lsl #2]
        beq     .LBB0_14
        ldrd    r1, r2, [r6, #-24]
        ldr.w   r3, [r4, r2, lsl #2]
        adds    r3, #1
        str.w   r3, [r4, r2, lsl #2]
        ldr.w   r3, [r9, r1, lsl #2]
        str     r2, [r0, #16]!
        str     r3, [r0, #4]
        str.w   r0, [r9, r1, lsl #2]
.LBB0_14:
        movs    r0, #0
        cmp.w   r8, #0
        str     r0, [r4]
        beq     .LBB0_17
        cmp.w   r8, #4
        and     r12, r8, #3
        bhs     .LBB0_18
        mov     r2, r8
        b       .LBB0_27
.LBB0_17:
        movs    r5, #1
        b       .LBB0_40
.LBB0_18:
        add.w   r0, r4, r8, lsl #2
        and     r3, r8, #252
        subs    r5, r0, #4
        movs    r0, #0
        mov     r2, r8
        b       .LBB0_21
.LBB0_19:
        subs    r7, r2, #2
        str.w   r7, [r4, r0, lsl #2]
        mov     r0, r7
        ldr     r7, [r5, #-8]
        cbz     r7, .LBB0_25
.LBB0_20:
        subs    r2, #4
        subs    r3, #4
        mov     r5, r1
        beq     .LBB0_26
.LBB0_21:
        ldr     r1, [r5, #4]
        cmp     r1, #0
        itt     eq
        streq.w r2, [r4, r0, lsl #2]
        moveq   r0, r2
        mov     r1, r5
        ldr     r7, [r1], #-16
        cbz     r7, .LBB0_23
        ldr     r7, [r5, #-4]
        cbnz    r7, .LBB0_24
        b       .LBB0_19
.LBB0_23:
        subs    r7, r2, #1
        str.w   r7, [r4, r0, lsl #2]
        mov     r0, r7
        ldr     r7, [r5, #-4]
        cmp     r7, #0
        beq     .LBB0_19
.LBB0_24:
        ldr     r7, [r5, #-8]
        cmp     r7, #0
        bne     .LBB0_20
.LBB0_25:
        subs    r7, r2, #3
        str.w   r7, [r4, r0, lsl #2]
        mov     r0, r7
        subs    r2, #4
        subs    r3, #4
        mov     r5, r1
        bne     .LBB0_21
.LBB0_26:
        cmp.w   r12, #0
        beq     .LBB0_30
.LBB0_27:
        ldr.w   r1, [r4, r2, lsl #2]
        cmp     r1, #0
        itt     eq
        streq.w r2, [r4, r0, lsl #2]
        moveq   r0, r2
        cmp.w   r12, #1
        beq     .LBB0_30
        subs    r1, r2, #1
        ldr.w   r3, [r4, r1, lsl #2]
        cmp     r3, #0
        ite     eq
        streq.w r1, [r4, r0, lsl #2]
        movne   r1, r0
        cmp.w   r12, #2
        bne     .LBB0_38
        mov     r0, r1
.LBB0_30:
        ldr     r1, [r4]
        cbz     r1, .LBB0_39
.LBB0_31:
        movs    r2, #0
.LBB0_32:
        ldr.w   r3, [r9, r1, lsl #2]
        str.w   r1, [r11, r2, lsl #2]
        cbnz    r3, .LBB0_36
.LBB0_33:
        subs.w  r8, r8, #1
        clz     r3, r8
        lsr.w   r5, r3, #5
        beq     .LBB0_40
        ldr.w   r1, [r4, r1, lsl #2]
        adds    r2, #1
        cmp     r1, #0
        bne     .LBB0_32
        b       .LBB0_40
.LBB0_35:
        ldr     r3, [r3, #4]
        cmp     r3, #0
        beq     .LBB0_33
.LBB0_36:
        ldr     r7, [r3]
        ldr.w   r6, [r4, r7, lsl #2]
        subs    r6, #1
        str.w   r6, [r4, r7, lsl #2]
        ldr     r5, [r3]
        ldr.w   r7, [r4, r5, lsl #2]
        cmp     r7, #0
        bne     .LBB0_35
        str.w   r5, [r4, r0, lsl #2]
        ldr     r0, [r3]
        b       .LBB0_35
.LBB0_38:
        subs    r0, r2, #2
        ldr.w   r2, [r4, r0, lsl #2]
        cmp     r2, #0
        ite     eq
        streq.w r0, [r4, r1, lsl #2]
        movne   r0, r1
        ldr     r1, [r4]
        cmp     r1, #0
        bne     .LBB0_31
.LBB0_39:
        movs    r5, #0
.LBB0_40:
        mov     r0, r4
        bl      asm_balloc_free
        mov     r0, r5
        add     sp, #20
        pop.w   {r4, r5, r6, r7, r8, r9, r10, r11, pc}
