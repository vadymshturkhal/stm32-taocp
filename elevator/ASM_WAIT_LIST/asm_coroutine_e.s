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

	.global ASM_E2
	.type ASM_E2, %function
	
	.global ASM_E3
	.type ASM_E3, %function

	.global ASM_E4A
	.type ASM_E4A, %function

	.global ASM_E4
	.type ASM_E4, %function

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

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ R7 uint32_t delay					@ FIXME: guarantee
ASM_E2A:
	LDR R3, =ASM_E2
	MOV R0, R11
	MOV R1, R8
	MOV R2, R7
	BL holdc						@ JMP HOLDC
	B ASM_CYCLE
	@ holdc(shared_state, C, delay, E2);

@ [Change of state?]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ Can use R7

@ Runtime:
@ R0 Accumulator after ASM_E2
ASM_E2:
	LDR R0, [R10, #STATE]
	CMP R0, #0
	BLT ASM_E2_1H					@ STATE is GOINGDOWN

@ [STATE is GOINGUP]
@ Are there calls for higher floors?
@ Runtime:
@ R3 &CALLS[FLOOR]
@ R4 &CALLS[j], from FLOORS to FLOOR
@ R5 &CALLS[0]
ASM_E2_HIGHER_CALLS:
	LDR R1, [R10, #FLOOR]
	
	ADD R5, R11, #CALLS							@ R5 = &CALLS[0]
	ADD R3, R5, R1, LSL #2						@ R3 = &CALLS[FLOOR] = Current floor CALLS
	ADD R4, R11, #(CALLS + (FLOORS - 1) * 4)	@ R4 = &CALLS[FLOORS - 1] = Higher floor CALLS

	MOVS R0, #0							@ R0 = rA = Accumulator

	CMP R4, R3
	BLE ASM_E2_HIGHER_DONE

ASM_E2_HIGHER_LOOP:
	LDR R2, [R4], #-4					@ sizeof CALLS is uint32_t
	ADDS R0, R0, R2						@ Add CALLS[j] value
	CMP R4, R3
	BNE ASM_E2_HIGHER_LOOP

@ If yes, go to ASM_E3
ASM_E2_HIGHER_DONE:
	CMP R0, #0
	BGT ASM_E3							@ JAP E3

@ Have passengers in the elevator called for lower floors?
@ Runtime:
@ R3 = &CALLS[FLOOR]
@ R5 = &CALLS[0]
ASM_E2_ELEVATOR_LOWER_FLOORS:
	MOVS R0, #0							@ R0 = rA = Accumulator

	CMP R5, R3
	BGE ASM_E2_LOWER_DONE				@ FLOOR == 0: no lower floors exist

ASM_E2_LOWER_LOOP:
	LDR R2, [R5], #4					@ sizeof CALLS is uint32_t
	ANDS R2, R2, #CALLCAR
	ORRS R0, R0, R2						@ rA |= CALLS[j] & CALLCAR
	CMP R5, R3
	BNE ASM_E2_LOWER_LOOP

@ If yes, reverse STATE; else set STATE to NEUTRAL
ASM_E2_LOWER_DONE:
	B ASM_E2_2H							@ JMP 2F

@ [STATE is GOINGDOWN]
@ Are there calls for lower floors?
@ Runtime:
@ R3 &CALLS[FLOOR]
@ R5 &CALLS[j], from 0 to FLOOR
ASM_E2_1H:
	LDR R1, [R10, #FLOOR]

	ADD R5, R11, #CALLS					@ R5 = &CALLS[0]
	ADD R3, R5, R1, LSL #2				@ R3 = &CALLS[FLOOR] = Current floor CALLS

	MOVS R0, #0							@ R0 = rA = Accumulator

	CMP R5, R3
	BGE ASM_E2_ELEVATOR_HIGHER_FLOORS				@ FLOOR == 0: no lower floors exist

ASM_E2_LOWER_CALLCAR_LOOP:
	LDR R2, [R5], #4					@ sizeof CALLS is uint32_t
	ADDS R0, R0, R2						@ Add CALLS[j] value
	CBNZ R0, ASM_E3						@ Jump to ASM_E3 if found lower call
	CMP R5, R3
	BNE ASM_E2_LOWER_CALLCAR_LOOP

@ Have passengers in the elevator called for higher floors?
@ Runtime:
@ R3 &CALLS[FLOOR] = Current floor CALLS
@ R4 &CALLS[FLOORS - 1]
ASM_E2_ELEVATOR_HIGHER_FLOORS:
	ADD R4, R11, #(CALLS + (FLOORS - 1) * 4)	@ R4 = &CALLS[FLOORS - 1] = Higher floor CALLS
	MOVS R0, #0							@ R0 = rA = Accumulator

	CMP R4, R3
	BLE ASM_E2_2H

ASM_E2_HIGHER_CALLCAR_LOOP:
	LDR R2, [R4], #-4					@ sizeof CALLS is uint32_t
	ANDS R2, R2, #CALLCAR
	ORRS R0, R0, R2						@ rA |= CALLS[j] & CALLCAR
	CBNZ R0, ASM_E2_2H					@ Jump to ASM_E2_2H if found higher call
	CMP R4, R3
	BNE ASM_E2_HIGHER_CALLCAR_LOOP

@ [Reverse direction of STATE]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R0 CALLS
@ R3 &CALLS[FLOOR] = Current floor CALLS
ASM_E2_2H:
	LDR R1, [R10, #STATE]
	RSBS R1, R1, #0						@ Reverse direction of STATE
	STR R1, [R10, #STATE]

	@ Set all CALL variables for the current FLOOR to zero
	MOVS R2, #0
	STR R2, [R3]						@ shared_state->CALLS[elevator->FLOOR] = 0;

	@ If called to the opposite direction: jump to E3
	CMP R0, #0
	BNE ASM_E3

	@ Otherwise set STATE to NEUTRAL
	STR R2, [R10, #STATE]

@ [Open door]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R4 = elevator->ELEV3
@ R7 uint32_t delay
ASM_E3:
	LDR R4, [R10, #ELEV3]					@ R4 = elevator->ELEV3

	@ if (elevator->ELEV3->left1 != NULL) elevator_list_delete_nodew(elevator->ELEV3);
	LDR R0, [R4, #LEFT1]
	CBZ R0, ASM_E3_SCHEDULE_E9

	MOVS R0, R4
	BL elevator_list_delete_nodew

@ Schedule activity E9 after 300 units
ASM_E3_SCHEDULE_E9:
	MOV R0, R11
	MOVS R1, R4
	MOV R2, #300
	BL hold									@ hold(shared_state, elevator->ELEV3, 300)

	MOV R0, R11
	LDR R1, [R10, #ELEV2]
	MOV R2, #76
	BL hold									@ hold(shared_state, elevator->ELEV2, 76);

	MOVS R0, #1
	STR R0, [R10, #D2]						@ elevator->D2 = 1;
	STR R0, [R10, #D1]						@ elevator->D1 = 1;

	MOV R7, #20								@ delay = 20

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ R7 uint32_t delay
ASM_E4A:
	LDR R3, =ASM_E4
	MOV R0, R11
	LDR R1, [R10, #ELEV1]
	MOV R2, R7
	BL holdc								@ holdc(shared_state, elevator->ELEV1, delay, ASM_E4)
	B ASM_CYCLE

@ [Let people out, in]
ASM_E4:
	MOVS R0, #3								@ TODO: port E4

ASM_E5A:
	MOVS R0, #3

ASM_E5:
	MOVS R0, #3

ASM_E6:
	MOVS R0, #3

ASM_E9:
	MOVS R0, #3
