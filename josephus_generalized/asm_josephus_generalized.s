.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_josephus_generalized


@ Input
@ R0 uint32_t mod;
@ R1 uint32_t participants;

@ Runtime
@ R2 D;
@ R3 (mod-1)*participants);
@ R4 mod-1;

@ Return
@ R0 josephus number;
asm_josephus_generalized:
	PUSH {R4, LR}

	MOVS R2, #1			@ D = 1
	SUBS R4, R0, #1		@ mod - 1
	MUL R3, R4, R1		@ (mod-1)*participants

	@ init float part
	VMOV S0, R0			@ mod
	VMOV S1, R4			@ mod - 1
	VMOV S2, R2			@ D
	VMOV S6, R3

	@ to float
	VCVT.F32.S32 S0, S0
	VCVT.F32.S32 S1, S1
	VCVT.F32.S32 S2, S2		@ D to float
	VCVT.F32.S32 S6, S6

	VMOV.F32 S4, #1.0		@ load constant 1
	VDIV.F32 S5, S0, S1		@ load constant mod / (mod - 1)

early_return:
	VCMP.F32 S2, S6
	VMRS APSR_nzcv, FPSCR
	BGT done

ceil_loop:
	VMUL.F32 S0, S5, S2		@ mod / (mod - 1) * D

	VCVT.S32.F32 S2, S0		@ truncate
	VCVT.F32.S32 S2, S2		@ convert to float
	VCMP.F32 S0, S2			@ compare original value with truncated one
	VMRS APSR_nzcv, FPSCR	@ FPU flags to CPU APSR

	IT GT
	VADDGT.F32 S2, S2, S4	@ add 1 if truncated < original

	@ comparing
	VCMP.F32 S2, S6
	VMRS APSR_nzcv, FPSCR
	BLS ceil_loop

josephus_value:
	@ return mod*participants + 1 - D;
	VCVT.S32.F32 S0, S2		@ convert D to integer
	VMOV R2, S0 			@ D
	MUL R0, R0, R1
	ADDS R0, R0, #1
	SUBS R0, R0, R2

done:
	POP {R4, PC}
