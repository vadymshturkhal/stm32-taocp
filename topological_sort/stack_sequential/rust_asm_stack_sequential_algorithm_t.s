.syntax unified
    .thumb
    .cpu cortex-m4
    .global rust_asm_stack_sequential_algorithm_t
    .type rust_asm_stack_sequential_algorithm_t, %function


__rustc_rust_begin_unwind:
        push    {r7, lr}
        mov     r7, sp
.LBB0_1:
        b       .LBB0_1

rust_asm_stack_sequential_algorithm_t:
        push    {r4, r5, r6, r7, lr}
        add     r7, sp, #12
        push.w  {r8, r9, r10, r11}
        sub     sp, #12
        add.w   r8, r0, #1
        mov     r4, r0
        add.w   r0, r8, r2
        lsls    r0, r0, #3
        mov     r9, r3
        mov     r6, r2
        mov     r10, r1
        bl      asm_balloc
        cbz     r0, .LBB1_4
        lsl.w   r11, r8, #2
        mov     r1, r11
        mov     r5, r0
        str.w   r9, [sp, #8]
        add.w   r9, r0, r8, lsl #2
        add.w   r8, r0, r8, lsl #3
        bl      __aeabi_memclr4
        mov     r0, r9
        mov     r1, r11
        movs    r2, #255
        bl      __aeabi_memset4
        cmp     r6, #0
        beq.w   .LBB1_11
        mvn     r1, #7
        add.w   r2, r1, r6, lsl #3
        movs    r1, #1
        add.w   r1, r1, r2, lsr #3
        add.w   r0, r10, r6, lsl #3
        cmp     r2, #24
        and     r12, r1, #3
        bhs     .LBB1_5
        mov.w   r11, #0
        b       .LBB1_8
.LBB1_4:
        movs    r0, #0
        add     sp, #12
        pop.w   {r8, r9, r10, r11}
        pop     {r4, r5, r6, r7, pc}
.LBB1_5:
        bic     lr, r1, #3
        add.w   r10, r5, r4, lsl #3
        movs    r6, #0
        str     r4, [sp, #4]
.LBB1_6:
        ldr     r1, [r0, #-4]
        ldr     r3, [r0, #-8]
        ldr.w   r2, [r5, r1, lsl #2]
        add.w   r11, r6, #4
        adds    r2, #1
        str.w   r2, [r5, r1, lsl #2]
        ldr.w   r4, [r9, r3, lsl #2]
        add.w   r2, r10, r6, lsl #3
        strd    r1, r4, [r2, #8]
        str.w   r6, [r9, r3, lsl #2]
        ldr     r1, [r0, #-12]
        ldr     r4, [r0, #-16]
        ldr.w   r3, [r5, r1, lsl #2]
        cmp     r11, lr
        add.w   r3, r3, #1
        str.w   r3, [r5, r1, lsl #2]
        ldr.w   r3, [r9, r4, lsl #2]
        strd    r1, r3, [r2, #16]
        add.w   r1, r6, #1
        str.w   r1, [r9, r4, lsl #2]
        ldr     r1, [r0, #-20]
        ldr     r4, [r0, #-24]
        ldr.w   r3, [r5, r1, lsl #2]
        add.w   r3, r3, #1
        str.w   r3, [r5, r1, lsl #2]
        ldr.w   r3, [r9, r4, lsl #2]
        strd    r1, r3, [r2, #24]
        add.w   r1, r6, #2
        str.w   r1, [r9, r4, lsl #2]
        ldr     r3, [r0, #-32]!
        ldr     r4, [r0, #4]
        ldr.w   r1, [r5, r4, lsl #2]
        add.w   r1, r1, #1
        str.w   r1, [r5, r4, lsl #2]
        ldr.w   r1, [r9, r3, lsl #2]
        strd    r4, r1, [r2, #32]
        add.w   r1, r6, #3
        mov     r6, r11
        str.w   r1, [r9, r3, lsl #2]
        bne     .LBB1_6
        ldr     r4, [sp, #4]
        cmp.w   r12, #0
        beq     .LBB1_11
.LBB1_8:
        ldrd    r2, r3, [r0, #-8]
        cmp.w   r12, #1
        ldr.w   r6, [r5, r3, lsl #2]
        add.w   r6, r6, #1
        str.w   r6, [r5, r3, lsl #2]
        ldr.w   lr, [r9, r2, lsl #2]
        add.w   r6, r8, r11, lsl #3
        str.w   r3, [r8, r11, lsl #3]
        str.w   lr, [r6, #4]
        str.w   r11, [r9, r2, lsl #2]
        beq     .LBB1_11
        ldrd    r2, r3, [r0, #-16]
        add.w   lr, r11, #1
        ldr.w   r6, [r5, r3, lsl #2]
        cmp.w   r12, #2
        add.w   r6, r6, #1
        str.w   r6, [r5, r3, lsl #2]
        ldr.w   r10, [r9, r2, lsl #2]
        add.w   r6, r8, lr, lsl #3
        str.w   r3, [r8, lr, lsl #3]
        str.w   r10, [r6, #4]
        str.w   lr, [r9, r2, lsl #2]
        beq     .LBB1_11
        ldrd    r0, r2, [r0, #-24]
        add.w   r1, r11, #2
        ldr.w   r3, [r5, r2, lsl #2]
        add.w   r6, r8, r1, lsl #3
        adds    r3, #1
        str.w   r3, [r5, r2, lsl #2]
        ldr.w   r3, [r9, r0, lsl #2]
        str.w   r2, [r8, r1, lsl #3]
        str     r3, [r6, #4]
        str.w   r1, [r9, r0, lsl #2]
.LBB1_11:
        mov     r10, r4
        cbz     r4, .LBB1_19
        ldr     r4, [sp, #8]
        movs    r0, #0
        mov     r1, r10
.LBB1_13:
        ldr.w   r3, [r5, r1, lsl #2]
        subs    r2, r1, #1
        it      eq
        moveq   r2, r1
        cmp     r3, #0
        itt     eq
        streq.w r0, [r5, r1, lsl #2]
        moveq   r0, r1
        cmp     r1, #1
        beq     .LBB1_15
        cmp     r2, #0
        mov     r1, r2
        bne     .LBB1_13
.LBB1_15:
        movs    r6, #0
        cbnz    r0, .LBB1_17
        b       .LBB1_20
.LBB1_16:
        add.w   r6, r6, #1
        cbz     r0, .LBB1_20
.LBB1_17:
        str.w   r0, [r4, r6, lsl #2]
        ldr.w   r1, [r9, r0, lsl #2]
        ldr.w   r0, [r5, r0, lsl #2]
        adds    r2, r1, #1
        beq     .LBB1_16
.LBB1_18:
        ldr.w   r2, [r8, r1, lsl #3]
        add.w   r1, r8, r1, lsl #3
        ldr.w   r3, [r5, r2, lsl #2]
        subs    r3, #1
        it      eq
        moveq   r3, r0
        str.w   r3, [r5, r2, lsl #2]
        ldr     r1, [r1, #4]
        it      eq
        moveq   r0, r2
        adds    r2, r1, #1
        bne     .LBB1_18
        b       .LBB1_16
.LBB1_19:
        movs    r6, #0
.LBB1_20:
        mov     r0, r5
        bl      asm_balloc_free
        sub.w   r0, r6, r10
        clz     r0, r0
        lsrs    r0, r0, #5
        add     sp, #12
        pop.w   {r8, r9, r10, r11}
        pop     {r4, r5, r6, r7, pc}
