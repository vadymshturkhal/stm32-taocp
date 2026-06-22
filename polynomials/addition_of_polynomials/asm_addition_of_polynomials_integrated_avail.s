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
@ R0 PolynomialCircularList* polynomial_P, P->ABC, scratch
@ R1 PolynomialCircularList* polynomial_Q, Q->ABC, scratch
@ R2 scratch
@ R3 scratch
@ R4 P
@ R5 Q
@ R6 Q1
@ R7 AVAIL
@ R8 polynomial_Q

@ Return: status

asm_addition_of_polynomials_integrated_avail:
	PUSH {R4-R8, LR}

	@ Storage_Pool* storage_pool = polynomial_Q->storage_pool;
	LDR R2, [R1, #POLYNOMIAL_STORAGE_POOL]

	@ PolynomialNode* P = polynomial_P->ptr;
	LDR R4, [R0, #POLYNOMIAL_PTR]

	@ PolynomialNode* Q = polynomial_Q->ptr;
	LDR R5, [R1, #POLYNOMIAL_PTR]

	LDR R7, [R2, #STORAGE_POOL_AVAIL]
	MOVS R8, R1					@ save polynomial_Q

.balign 4
@ A1. [Initialize]
A1:
	LDR R4, [R4, #NODE_LINK]	@ P = P->link;
	MOVS R6, R5 				@ Q1 = Q;
	LDR R5, [R5, #NODE_LINK]	@ Q = Q->link;

init_A2:
	LDR R0, [R4, #NODE_ABC]

@ A2. [ABC(P):ABC(Q)]
.balign 4
A2:
	LDR R1, [R5, #NODE_ABC]

	CMP R0, R1
	BEQ A3
	BGT A5

	@ less than
	MOVS R6, R5 				@ Q1 = Q;
	LDR R5, [R5, #NODE_LINK]	@ Q = Q->link;
	B A2

@ A3. [Add coefficients]
A3:
	CMP R0, #0
	BLT done

	LDR R0, [R4, #NODE_COEFF]	@ P->COEFF
	LDR R1, [R5, #NODE_COEFF]	@ Q->COEFF

	ADDS R0, R0, R1				@ P->COEFF + Q->COEFF
	STR R0, [R5, #NODE_COEFF]	@ Q->COEFF = P->COEFF + Q->COEFF
	BNE A1						@ if not zero go to A1

@ A4. [Delete zero term]
A4:
	MOVS R1, R5					@ Q2 = Q
	LDR R5, [R5, #NODE_LINK]	@ Q = Q->link
	STR R5, [R6, #NODE_LINK]	@ Q1->link = Q

	LDR R0, [R8, #POLYNOMIAL_SIZE]	@ load polynomial size
	@ AVAIL <= Q2
	STR R7, [R1, #NODE_LINK]	@ Q2->link = AVAIL
	SUBS R0, R0, #1				@ polynomial size--;
	MOVS R7, R1					@ AVAIL = Q2

	LDR R4, [R4, #NODE_LINK]	@ P = P->link;
	STR R0, [R8, #POLYNOMIAL_SIZE]	@ store polynomial size;

	@ B A2
	B init_A2

@ A5. [Insert new term]
A5:
	@ Q2 <= AVAIL
	MOVS R0, R7
	CBZ R0, overflow
	LDR R7, [R7, #NODE_LINK]	@ R0 = Q

	LDR R1, [R4, #NODE_COEFF]	@ P->COEFF
	LDR R2, [R4, #NODE_ABC]		@ P->ABC
	STR R1, [R0, #NODE_COEFF]	@ Q->COEFF = P->COEFF
	STR R2, [R0, #NODE_ABC]		@ Q->ABC = P->ABC

	// LDMIA + STMIA are slower than 2 LDR + 2 STR

	LDR R1, [R8, #POLYNOMIAL_SIZE]	@ load polynomial size
	STR R5, [R0, #NODE_LINK]	@ Q2->link = Q;
	STR R0, [R6, #NODE_LINK]	@ Q1->link = Q2;
	ADDS R1, R1, #1				@ advance polynomial size

	LDR R4, [R4, #NODE_LINK]	@ P = P->link;

	@ polynomial_Q->size++;
	STR R1, [R8, #POLYNOMIAL_SIZE]	@ store polynomial size

	B init_A2 						@ Go to step 2

done:
	MOVS R0, #0
	POP {R4-R8, PC}

overflow:
	MOVS R0, #1
	POP {R4-R8, PC}
