.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_multiplication_of_polynomials_avail_integrated
	.type asm_multiplication_of_polynomials_avail_integrated, %function


@ PolynomialCircularList fields definition
.equ POLYNOMIAL_PTR, 			0
.equ POLYNOMIAL_STORAGE_POOL,	4
.equ POLYNOMIAL_SIZE,			8

@ PolynomialNode fields definition
.equ NODE_LINK, 				0
.equ NODE_COEFF, 				4
.equ NODE_ABC, 					8

@ Storage_Pool fields definition
.equ STORAGE_POOL_AVAIL,		0

.equ SENTINEL_NODE_ABC, -16777215

@ Input:
@ R0 PolynomialCircularList* polynomial_Q
@ R1 PolynomialCircularList* polynomial_M
@ R2 PolynomialCircularList* polynomial_P

@ Runtime:
@ R0 PolynomialCircularList* polynomial_Q
@ R1 PolynomialNode* M
@ R2 PolynomialCircularList* polynomial_P
@ R3 M->ABC
@ R4 PolynomialCircularList* polynomial_Q
@ R5 PolynomialNode* M
@ R6 PolynomialCircularList* polynomial_P

@ Result: Polynomial Q + Polynomial M x Polynomial P
@ Return: status
asm_multiplication_of_polynomials_avail_integrated:
	PUSH {R4-R6, LR}

	MOVS R4, R0
	MOVS R5, R1
	MOVS R6, R2

	LDR R5, [R5, #POLYNOMIAL_PTR]	@ PolynomialNode* M = polynomial_M->ptr

@ M1. [Next multiplier]
M1:
	LDR R5, [R5, #NODE_LINK]		@ M = M->link
	LDR R3, [R5, #NODE_ABC]			@ M->ABC

	CMP R3, #0
	BLT multiplication_of_polynomials_done

@ M2. [Multiply cycle]
M2:
	@ Init values for next iteration
	MOVS R0, R4						@ R0 polynomial_Q
	MOVS R1, R5						@ R5 = M->link
	MOVS R2, R6						@ R6 = polynomial_P

	@ Must be:
	@ R0 polynomial_Q
	@ R1 M
	@ R2 polynomial_P
	BL asm_multiply_cycle_avail_integrated
	CBNZ R0, multiplication_of_polynomials_error_exit	@ if (status != 0) return status

	@ Init values for next iteration
	@ MOVS R0, R4						@ R0 polynomial_Q
	@ MOVS R2, R6						@ R6 = polynomial_P

	B M1

multiplication_of_polynomials_done:
	MOVS R0, #0
	POP {R4-R6, PC}

multiplication_of_polynomials_error_exit:
	@ Return status
	POP {R4-R6, PC}
