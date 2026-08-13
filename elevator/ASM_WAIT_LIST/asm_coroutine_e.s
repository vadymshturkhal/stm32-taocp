.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text

	.global ASM_ELEVATOR_INIT
	.type ASM_ELEVATOR_INIT, %function

    .global ASM_E1A
	.type ASM_E1A, %function

    .global ASM_E1
	.type ASM_E1, %function

	.global ASM_E3
	.type ASM_E3, %function

	.global ASM_E5A
	.type ASM_E5A, %function

	.global ASM_E5
	.type ASM_E5, %function

	.global ASM_E6
	.type ASM_E6, %function

	.global ASM_E9
	.type ASM_E9, %function

@ ElevatorNode fields definition
.equ LEFT1, 			0
.equ RIGHT1,			4
.equ LEFT2, 			8
.equ RIGHT2,			12
.equ NEXTTIME, 			16
.equ NEXTINST,			20
.equ IN,				24
.equ OUT,				28
.equ GIVEUPTIME,		32

@ Elevator fields definition
.equ SHARED_STATE, 		0
.equ STATE,				4
.equ FLOOR, 			8
.equ D1,				12
.equ D2,	 			16
.equ D3,				20
.equ ELEV1,				24
.equ ELEV2,				28
.equ ELEV3,				32
.equ FIRST_SEARCH,		36

.equ HOME_FLOOR,		2

@ SharedState fields definition
.equ TIME, 				0
.equ CALLS,				4
.equ ELEVATOR_LIST, 	24
.equ QUEUE,				32
.equ WAIT_LIST,	 		72
.equ ELEVATOR,			80
.equ USERS,				84

@ uint32_t elevator_init(Elevator* elevator, SharedState* shared_state, Storage_Pool* storage_pool)
@ Input:
@ R0 Elevator* elevator
@ R1 SharedState* shared_state
@ R2 Storage_Pool* storage_pool

@ Runtime:
@ R4 = elevator
@ R5 = storage_pool

@ Output:
@ R0 uint32_t status (0 = OK, 1 = NULL arg, 2 = pool empty)
ASM_ELEVATOR_INIT:
	PUSH {R4-R6, LR}				@ Pushed R6 for 8-byte stack alignment

	@ if (elevator == NULL || shared_state == NULL || storage_pool == NULL) return 1;
	CMP R0, #0
	BEQ ELEVATOR_INIT_ERR1
	CMP R1, #0
	BEQ ELEVATOR_INIT_ERR1
	CMP R2, #0
	BEQ ELEVATOR_INIT_ERR1

	MOVS R4, R0						@ R4 = elevator
	MOVS R5, R2						@ R5 = storage_pool

	STR R1, [R4, #SHARED_STATE]		@ elevator->shared_state = shared_state;
	STR R0, [R1, #ELEVATOR]			@ shared_state->elevator = elevator;

	@ elevator->ELEV1 = storage_pool_pop(storage_pool);
	MOVS R0, R5
	BL storage_pool_pop
	CBZ R0, ELEVATOR_INIT_ERR2
	STR R0, [R4, #ELEV1]

	LDR R1, =ASM_E1
	STR R1, [R0, #NEXTINST]			@ elevator->ELEV1->NEXTINST = ASM_E1;

	@ elevator->ELEV2 = storage_pool_pop(storage_pool);
	MOVS R0, R5
	BL storage_pool_pop
	CBZ R0, ELEVATOR_INIT_ERR2
	STR R0, [R4, #ELEV2]

	LDR R1, =ASM_E5
	STR R1, [R0, #NEXTINST]			@ elevator->ELEV2->NEXTINST = ASM_E5;

	@ elevator->ELEV3 = storage_pool_pop(storage_pool);
	MOVS R0, R5
	BL storage_pool_pop
	CBZ R0, ELEVATOR_INIT_ERR2
	STR R0, [R4, #ELEV3]

	MOVS R1, #0
	STR R1, [R0, #LEFT1]			@ elevator->ELEV3->left1 = NULL; (Matches MIX memory starting zero)

	LDR R1, =ASM_E9
	STR R1, [R0, #NEXTINST]			@ elevator->ELEV3->NEXTINST = ASM_E9;

	MOVS R0, #0
	STR R0, [R4, #STATE]			@ elevator->STATE = 0; (Neutral)

	MOVS R1, #HOME_FLOOR
	STR R1, [R4, #FLOOR]			@ elevator->FLOOR = HOME_FLOOR;

	STR R0, [R4, #D1]				@ elevator->D1 = 0;
	STR R0, [R4, #D2]				@ elevator->D2 = 0;
	STR R0, [R4, #D3]				@ elevator->D3 = 0;
	STRB R0, [R4, #FIRST_SEARCH]	@ elevator->first_search = false;

	POP {R4-R6, PC}					@ R0 is 0: return 0

ELEVATOR_INIT_ERR1:
	MOVS R0, #1
	POP {R4-R6, PC}

ELEVATOR_INIT_ERR2:
	MOVS R0, #2
	POP {R4-R6, PC}

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E1A:
	LDR R0, [R10, #ELEV1]
	LDR R1, =ASM_E1
	BL cycle1						@ JMP CYCLE1

@ [Wait for call]
ASM_E1:
	B ASM_CYCLE

ASM_E3:
	MOVS R0, #3

ASM_E5A:
	MOVS R0, #3

ASM_E5:
	MOVS R0, #3

ASM_E6:
	MOVS R0, #3

ASM_E9:
	MOVS R0, #3
