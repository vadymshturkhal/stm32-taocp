.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global asm_addition_of_polynomials_integrated_avail
	.type asm_addition_of_polynomials_integrated_avail, %function


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

@ Input:
@ R0 PolynomialCircularList* polynomial_P
@ R1 PolynomialCircularList* polynomial_Q

@ Runtime:
@ R0 PolynomialCircularList* polynomial_P, PolynomialNode* P
@ R1 PolynomialCircularList* polynomial_Q
@ R2 scratch
@ R3 scratch
@ R4 AVAIL
@ R5 Q
@ R6 Q1

@ Return: status

asm_addition_of_polynomials_integrated_avail:
	PUSH {R4-R6, LR}

	LDR R2, [R1, #POLYNOMIAL_STORAGE_POOL]	@ Load polynomial_Q->storage_pool
	LDR R0, [R0, #POLYNOMIAL_PTR]			@ P = polynomial_P->ptr
	LDR R5, [R1, #POLYNOMIAL_PTR]			@ Q = polynomial_Q->ptr
	LDR R4, [R2, #STORAGE_POOL_AVAIL]		@ Load AVAIL

.balign 4
@ A1. [Initialize]
A1:
	LDR R0, [R0, #NODE_LINK]	@ P = P->link
	MOVS R6, R5 				@ Q1 = Q
	LDR R5, [R5, #NODE_LINK]	@ Q = Q->link

init_A2:
	LDR R2, [R0, #NODE_ABC]		@ P->ABC

@ A2. [ABC(P):ABC(Q)]
.balign 4
A2:
	LDR R3, [R5, #NODE_ABC]		@ Q->ABC

	CMP R2, R3
	BEQ A3
	BGT A5

	@ less than
	MOVS R6, R5 				@ Q1 = Q
	LDR R5, [R5, #NODE_LINK]	@ Q = Q->link
	B A2

@ A3. [Add coefficients]
A3:
	CMP R2, #0
	BLT done

	LDR R2, [R0, #NODE_COEFF]	@ P->COEFF
	LDR R3, [R5, #NODE_COEFF]	@ Q->COEFF

	ADDS R3, R3, R2				@ P->COEFF + Q->COEFF
	STR R3, [R5, #NODE_COEFF]	@ Q->COEFF = P->COEFF + Q->COEFF
	BNE A1						@ if not zero go to A1

@ A4. [Delete zero term]
A4:
	MOVS R2, R5						@ Q2 = Q
	LDR R5, [R5, #NODE_LINK]		@ Q = Q->link
	STR R5, [R6, #NODE_LINK]		@ Q1->link = Q

	LDR R3, [R1, #POLYNOMIAL_SIZE]	@ load polynomial size

	@ AVAIL <= Q2
	STR R4, [R2, #NODE_LINK]		@ Q2->link = AVAIL
	SUBS R3, R3, #1					@ polynomial size--
	MOVS R4, R2						@ AVAIL = Q2

	LDR R0, [R0, #NODE_LINK]		@ P = P->link;
	STR R3, [R1, #POLYNOMIAL_SIZE]	@ store polynomial size

	@ B A2
	B init_A2

@ A5. [Insert new term]
A5:
	@ Q2 <= AVAIL
	MOVS R3, R4					@ R3 = Q
	CBZ R3, overflow
	LDR R4, [R4, #NODE_LINK]	@ AVAIL = AVAIL->link

	STR R2, [R3, #NODE_ABC]		@ Q->ABC = P->ABC (R2 is P->ABC after A2)
	LDR R2, [R0, #NODE_COEFF]	@ P->COEFF
	STR R2, [R3, #NODE_COEFF]	@ Q->COEFF = P->COEFF

	// LDMIA + STMIA are slower than 2 LDR + 2 STR

	LDR R2, [R1, #POLYNOMIAL_SIZE]	@ load polynomial size
	STR R5, [R3, #NODE_LINK]		@ Q2->link = Q;
	STR R3, [R6, #NODE_LINK]		@ Q1->link = Q2;
	ADDS R2, R2, #1					@ advance polynomial size

	LDR R0, [R0, #NODE_LINK]		@ P = P->link;

	STR R2, [R1, #POLYNOMIAL_SIZE]	@ store polynomial size

	B init_A2 						@ Go to step 2

done:
	MOVS R0, #0
	B exit

overflow:
	MOVS R0, #1

exit:
	LDR R2, [R1, #POLYNOMIAL_STORAGE_POOL]	@ Load storage_pool
	STR R4,	[R2, #STORAGE_POOL_AVAIL]		@ Store AVAIL
	POP {R4-R6, PC}
