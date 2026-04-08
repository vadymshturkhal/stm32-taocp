.syntax unified
    .thumb
    .cpu cortex-m4

    .global rust_asm_perform_queue_operations_gem2

    .extern asm_balloc
    .extern asm_balloc_free

    .section .text
    .balign 4


@ Stats with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
@ cycles_cold = [2883], cycles_warm = [2670], size = 588 bytes


__rustc_rust_begin_unwind:
        push    {r7, lr}
        mov     r7, sp
.LBB0_1:
        b       .LBB0_1

rust_create_queue:
        push    {r4, r6, r7, lr}
        add     r7, sp, #8
        add.w   r3, r0, #12
        mov.w   r12, #0
        cmp     r1, #2
        mov     r4, r3
        strd    r12, r0, [r0]
        blo     .LBB1_7
        and     r4, r1, #3
        cmp     r4, #1
        sub.w   lr, r1, #2
        bne     .LBB1_3
        mov     r2, r3
        cmp.w   lr, #3
        bhs     .LBB1_6
        b       .LBB1_7
.LBB1_3:
        add.w   r2, r0, #20
        cmp     r4, #2
        str     r2, [r0, #16]
        bne     .LBB1_5
        subs    r1, #1
        mov     r4, r2
        cmp.w   lr, #3
        bhs     .LBB1_6
        b       .LBB1_7
.LBB1_5:
        add.w   r2, r0, #28
        mvns    r4, r1
        lsls    r4, r4, #30
        str     r2, [r0, #24]
        iteee   eq
        moveq   r1, lr
        addne.w r2, r0, #36
        strne   r2, [r0, #32]
        subne   r1, #3
        mov     r4, r2
        cmp.w   lr, #3
        blo     .LBB1_7
.LBB1_6:
        add.w   r4, r2, #8
        str     r4, [r2, #4]
        add.w   r4, r2, #16
        str     r4, [r2, #12]
        add.w   r4, r2, #24
        str     r4, [r2, #20]
        add.w   r4, r2, #32
        subs    r1, #4
        str     r4, [r2, #28]
        cmp     r1, #1
        mov     r2, r4
        bhi     .LBB1_6
.LBB1_7:
        movs    r1, #1
        strd    r1, r12, [r4]
        str     r3, [r0, #8]
        pop     {r4, r6, r7, pc}

rust_asm_perform_queue_operations_gem2:
        push    {r4, r5, r6, r7, lr}
        add     r7, sp, #12
        str     r8, [sp, #-4]!
        cbz     r0, .LBB2_6
        mov     r8, r0
        movs    r0, #12
        add.w   r0, r0, r8, lsl #3
        bl      asm_balloc
        mov.w   r5, #0
        cbz     r0, .LBB2_7
        add.w   r12, r0, #12
        cmp.w   r8, #1
        mov     r1, r12
        strd    r5, r0, [r0]
        beq     .LBB2_12
        and     r5, r8, #3
        cmp     r5, #1
        mov     r3, r8
        mov     r2, r12
        beq     .LBB2_10
        add.w   r2, r0, #20
        cmp     r5, #2
        str     r2, [r0, #16]
        bne     .LBB2_8
        sub.w   r3, r8, #1
        b       .LBB2_9
.LBB2_6:
        movs    r5, #0
.LBB2_7:
        mov     r0, r5
        ldr     r8, [sp], #4
        pop     {r4, r5, r6, r7, pc}
.LBB2_8:
        add.w   r2, r0, #28
        mvn.w   r1, r8
        lsls    r1, r1, #30
        str     r2, [r0, #24]
        iteee   eq
        subeq.w r3, r8, #2
        addne.w r2, r0, #36
        strne   r2, [r0, #32]
        subne.w r3, r8, #3
.LBB2_9:
        mov     r1, r2
.LBB2_10:
        sub.w   r6, r8, #2
        cmp     r6, #3
        blo     .LBB2_12
.LBB2_11:
        add.w   r1, r2, #8
        str     r1, [r2, #4]
        add.w   r1, r2, #16
        str     r1, [r2, #12]
        add.w   r1, r2, #24
        str     r1, [r2, #20]
        add.w   r1, r2, #32
        subs    r3, #4
        str     r1, [r2, #28]
        cmp     r3, #1
        mov     r2, r1
        bhi     .LBB2_11
.LBB2_12:
        movs    r2, #1
        movs    r3, #0
        strd    r2, r3, [r1]
        ldr     r2, [r0, #4]
        sub.w   r3, r8, #3
        str.w   r12, [r0, #8]
.LBB2_13:
        cmp.w   r12, #0
        beq     .LBB2_32
        mov     r1, r12
        ldr     r4, [r1, #4]!
        adds    r6, r3, #3
        adds    r5, r3, #2
        str.w   r6, [r12]
        str.w   r12, [r2]
        beq     .LBB2_21
        cmp     r4, #0
        beq     .LBB2_32
        mov     r2, r4
        ldr     r12, [r2, #4]!
        adds    r6, r3, #1
        str     r5, [r4]
        str     r4, [r1]
        bhs     .LBB2_23
        cmp.w   r12, #0
        beq     .LBB2_32
        mov     r1, r12
        ldr     r5, [r1, #4]!
        str.w   r6, [r12]
        str.w   r12, [r2]
        cbz     r3, .LBB2_22
        cbz     r5, .LBB2_32
        mov     r2, r5
        ldr     r12, [r2, #4]!
        str     r3, [r5]
        subs    r3, #4
        adds    r6, r3, #3
        str     r5, [r1]
        bne     .LBB2_13
        b       .LBB2_23
.LBB2_21:
        mov     r2, r1
        mov     r12, r4
        b       .LBB2_23
.LBB2_22:
        mov     r2, r1
        mov     r12, r5
.LBB2_23:
        movs    r1, #0
        str     r1, [r2]
        ldr     r1, [r0]
        strd    r2, r12, [r0, #4]
.LBB2_24:
        cbz     r1, .LBB2_32
        ldr     r3, [r1, #4]
        cmp.w   r8, #1
        str.w   r12, [r1, #4]
        beq     .LBB2_33
        cbz     r3, .LBB2_32
        ldr     r2, [r3, #4]
        cmp.w   r8, #2
        str     r1, [r3, #4]
        beq     .LBB2_34
        cbz     r2, .LBB2_32
        ldr.w   r12, [r2, #4]
        cmp.w   r8, #3
        str     r3, [r2, #4]
        beq     .LBB2_35
        cmp.w   r12, #0
        beq     .LBB2_32
        ldr.w   r5, [r12, #4]
        subs.w  r8, r8, #4
        mov     r1, r5
        str.w   r2, [r12, #4]
        bne     .LBB2_24
        b       .LBB2_36
.LBB2_32:
        movs    r5, #0
        bl      asm_balloc_free
        mov     r0, r5
        ldr     r8, [sp], #4
        pop     {r4, r5, r6, r7, pc}
.LBB2_33:
        mov     r5, r3
        mov     r12, r1
        b       .LBB2_36
.LBB2_34:
        mov     r5, r2
        mov     r12, r3
        b       .LBB2_36
.LBB2_35:
        mov     r5, r12
        mov     r12, r2
.LBB2_36:
        cmp     r5, #0
        it      eq
        streq   r0, [r0, #4]
        str     r5, [r0]
        str.w   r12, [r0, #8]
        movs    r5, #1
        bl      asm_balloc_free
        mov     r0, r5
        ldr     r8, [sp], #4
        pop     {r4, r5, r6, r7, pc}
