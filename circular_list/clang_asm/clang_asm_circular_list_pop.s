.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global clang_asm_circular_list_pop
	.type clang_asm_circular_list_pop, %function


clang_asm_circular_list_pop:
        ldr     r3, [r1]
        cbz     r3, .LBB0_3
        ldr     r2, [r3, #4]
        cmp     r3, r2
        beq     .LBB0_4
        ldr     r0, [r2, #4]
        str     r0, [r3, #4]
        b       .LBB0_5
.LBB0_3:
        mov     r2, r0
        movs    r0, #0
        strb    r0, [r2]
        bx      lr
.LBB0_4:
        movs    r0, #0
        str     r0, [r1]
.LBB0_5:
        ldr     r0, [r1, #4]
        str     r2, [r1, #4]
        str     r0, [r2, #4]
        ldr     r0, [r2]
        bx      lr
