.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global ASM_DECISION
	.type ASM_DECISION, %function

@ SharedState fields definition
.equ TIME, 				0
.equ CALLS,				4
.equ ELEVATOR_LIST, 	24
.equ QUEUE,				32
.equ WAIT_LIST,	 		72
.equ ELEVATOR,			80
.equ USERS,				84

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

.equ HOME_FLOOR,		2
.equ FLOORS,			5

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users

@ Runtime
@ R8 elevator->ELEV1
@ R7 ASM_E3 or ASM_E6
ASM_DECISION:
    PUSH {R8, LR}                       @ Pushed R8 for 8-byte alignment

	@ D1. Decision necessary?
	LDR R0, [R10, #STATE]
	CMP R0, #0
	BNE ASM_DECISION_9H			        @ if (elevator->STATE != 0) return;

	@ D2. Should door open?
	LDR R8, [R10, #ELEV1]				@ R0 = ELEV1
	LDR R1, [R8, #NEXTINST]
	LDR R2, =ASM_E1
	CMP R1, R2
	BNE ASM_DECISION_1H				    @ ELEV1->NEXTINST != ASM_E1: skip

    @ Prepare to schedule ASM_E3
    LDR R7, =ASM_E3

    @ Magic 4 is uint_32
	LDR R1, [R11, #(CALLS + HOME_FLOOR * 4)]	@ R1 = shared_state->CALLS[HOME_FLOOR]
	CMP R1, #0
	BNE ASM_DECISION_8H				    @ CALLS[HOME_FLOOR] == 0: skip

@ D3. Any calls?
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 elevator->ELEV1
@ R7 ASM_E3 or ASM_E6

@ Runtime:
@ R6 j
@ R5 elevator->FLOOR
@ R4 = &shared_state->CALLS[0]
@ R3 i
ASM_DECISION_1H:
	MOVS R6, #-1						@ R6 = j = -1
	LDR R5, [R10, #FLOOR]				@ R5 = elevator->FLOOR
	ADD R4, R11, #CALLS					@ R4 = &shared_state->CALLS[0]
	MOVS R3, #0							@ i = 0

.balign 4
ASM_DECISION_1H_LOOP:
    @ if (i == elevator->FLOOR) continue;
	CMP R3, R5
    BEQ ASM_DECISION_1H_CONTINUE_LOOP

    @ if (shared_state->CALLS[i] == 0) continue;
	LDR R2, [R4, R3, LSL #2]			@ R2 = shared_state->CALLS[0]
	CMP R2, #0
	BEQ ASM_DECISION_1H_CONTINUE_LOOP

    @ j = i;
    MOVS R6, R3
    B ASM_DECISION_D3_DONE              @ break

ASM_DECISION_1H_CONTINUE_LOOP:
    @ Increment i
    ADDS R3, R3, #1

    @ Continue if i < FLOORS
    CMP R3, #FLOORS
    BNE ASM_DECISION_1H_LOOP

@ All CALLS[i], i != FLOOR, are zero
@ Input:
@ R6 j
ASM_DECISION_D3_DONE:
	CMP R6, #-1
	BNE ASM_DECISION_2H				@ j != -1: skip

    @ All CALL[j], j != elevator->FLOOR, are zero
	@ Is caller ASM_E6B?
	LDR R0, =ASM_E6B
	CMP LR, R0                      @ Compare with Link Register
	BNE ASM_DECISION_9H			    @ caller != ASM_E6: return

	MOVS R6, #HOME_FLOOR			@ j = HOME_FLOOR

@ D4. Set STATE
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 elevator->ELEV1
@ R7 ASM_E3 or ASM_E6
@ R6 j
@ R5 elevator->FLOOR
ASM_DECISION_2H:
    @ D4. Set STATE
	SUBS R0, R6, R5					@ R0 = j - FLOOR
    STR R0, [R10, #STATE]			@ elevator->STATE = j - FLOOR

	@ D5. Elevator dormant?
	LDR R1, [R8, #NEXTINST]
	LDR R2, =ASM_E1
	CMP R1, R2
	BNE ASM_DECISION_9H			    @ ELEV1->NEXTINST != ASM_E1: return

	CMP R0, #0
	BEQ ASM_DECISION_9H			    @ STATE == 0: return

	@ Otherwise schedule E6
	LDR R7, =ASM_E6

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 elevator->ELEV1
@ R7 ASM_E3 or ASM_E6
@ R6 j
@ R5 elevator->FLOOR
ASM_DECISION_8H:
	STR R7, [R8, #NEXTINST]			@ ELEV1->NEXTINST = ASM_E6

	MOVS R0, R11					@ R0 = shared_state
	MOVS R1, R8						@ R1 = ELEV1
	MOVS R2, #20					@ delay = 20
	BL hold							@ hold(shared_state, ELEV1, 20);

ASM_DECISION_9H:
    POP {R8, PC}
