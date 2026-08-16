.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text
    .global ASM_CYCLE_START
	.type ASM_CYCLE_START, %function

    .global ASM_CYCLE
	.type ASM_CYCLE, %function

    .global ASM_CYCLE_DONE
	.type ASM_CYCLE_DONE, %function

    .global ASM_DECISION
	.type ASM_DECISION, %function

    .global ASM_SORTIN
	.type ASM_SORTIN, %function

    .global ASM_HOLD
	.type ASM_HOLD, %function

    .global ASM_HOLDC
	.type ASM_HOLDC, %function

@ NOTE: R7-R11 are global registers which contain global state and don't need to PUSH them every step
@ NOTE: Using C NextInst with global parameters permanently stored in R8-R11 Registers

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

@ ElevatorList fields definition
@ .equ ELEVATOR_LIST_HEAD, 0
@ .equ ELEVATOR_LIST_STORAGE_POOL,4

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

@ Runtime: Set global variables
@ R0-R3 scratch
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_CYCLE_START:
	PUSH {R3-R11, LR}			@ Save all Registers and use R3 for 8-byte alignment
	MOVS R11, R0				@ R11 = shared_state
	LDR R10, [R0, #ELEVATOR]	@ R10 = elevator
	LDR R9, [R0, #USERS]		@ R9 = users
	B ASM_CYCLE

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 node C global

@ Runtime:
@ Save
@ R7 elevator->ELEV1
@ R6 ASM_E3 or ASM_E6
@ Scratch
@ R0 STATE, shared_state->CALLS[HOME_FLOOR]
@ R1 elevator->ELEV1->NEXTINST
@ R2 ASM_E1
ASM_DECISION:
	@ D1. Decision necessary?
	LDR R0, [R10, #STATE]
	CMP R0, #0
	BNE ASM_DECISION_9H			        @ if (elevator->STATE != 0) return;

	@ D2. Should door open?
	LDR R7, [R10, #ELEV1]				@ R7 = ELEV1
	LDR R1, [R7, #NEXTINST]
	LDR R2, =ASM_E1
	CMP R1, R2
	BNE ASM_DECISION_1H				    @ ELEV1->NEXTINST != ASM_E1: skip

    @ Prepare to schedule ASM_E3
    LDR R6, =ASM_E3

    @ Magic 4 is uint_32
	LDR R1, [R11, #(CALLS + HOME_FLOOR * 4)]	@ R1 = shared_state->CALLS[HOME_FLOOR]
	CMP R1, #0
	BNE ASM_DECISION_8H				    @ CALLS[HOME_FLOOR] == 0: skip

@ D3. Any calls?
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 user
@ R7 elevator->ELEV1
@ R6 ASM_E3 or ASM_E6

@ Runtime:
@ R5 j
@ R4 elevator->FLOOR
@ R3 = &shared_state->CALLS[0]
@ R2 i
@ R1 shared_state->CALLS[0]
ASM_DECISION_1H:
	MOVS R5, #-1						@ R5 = j = -1
	LDR R4, [R10, #FLOOR]				@ R4 = elevator->FLOOR
	ADD R3, R11, #CALLS					@ R3 = &shared_state->CALLS[0]
	MOVS R2, #0							@ R2 = i = 0

.balign 4
ASM_DECISION_1H_LOOP:
    @ if (i == elevator->FLOOR) continue;
	CMP R2, R4
    BEQ ASM_DECISION_1H_CONTINUE_LOOP

    @ if (shared_state->CALLS[i] == 0) continue;
	LDR R1, [R3, R2, LSL #2]			@ R1 = shared_state->CALLS[0]
	CMP R1, #0
	BEQ ASM_DECISION_1H_CONTINUE_LOOP

    @ j = i;
    MOVS R5, R2
    B ASM_DECISION_D3_DONE              @ break

ASM_DECISION_1H_CONTINUE_LOOP:
    @ Increment i
    ADDS R2, R2, #1

    @ Continue if i < FLOORS
    CMP R2, #FLOORS
    BNE ASM_DECISION_1H_LOOP

@ All CALLS[i], i != FLOOR, are zero
@ Input:
@ R5 j
ASM_DECISION_D3_DONE:
	CMP R5, #-1
	BNE ASM_DECISION_2H				@ j != -1: skip

    @ All CALL[j], j != elevator->FLOOR, are zero
	@ Is caller ASM_E6B?
	LDR R0, =ASM_E6B
	CMP LR, R0                      @ Compare with Link Register
	BNE ASM_DECISION_9H			    @ caller != ASM_E6: return

	MOVS R5, #HOME_FLOOR			@ j = HOME_FLOOR

@ D4. Set STATE
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 user
@ R7 elevator->ELEV1
@ R6 ASM_E3 or ASM_E6
@ R5 j
@ R4 elevator->FLOOR
ASM_DECISION_2H:
    @ D4. Set STATE
	SUBS R0, R5, R4					@ R0 = j - FLOOR
    STR R0, [R10, #STATE]			@ elevator->STATE = j - FLOOR

	@ D5. Elevator dormant?
	LDR R1, [R7, #NEXTINST]
	LDR R2, =ASM_E1
	CMP R1, R2
	BNE ASM_DECISION_9H			    @ ELEV1->NEXTINST != ASM_E1: return

	CMP R0, #0
	BEQ ASM_DECISION_9H			    @ STATE == 0: return

	@ Otherwise schedule E6
	LDR R6, =ASM_E6

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 user
@ R7 elevator->ELEV1
@ R6 ASM_E3 or ASM_E6

@ Runtime:
@ R6 delay
ASM_DECISION_8H:
	STR R6, [R7, #NEXTINST]			@ ELEV1->NEXTINST = ASM_E3 or ASM_E6

	MOVS R0, R7						@ R0 = node
	MOVS R1, #20					@ R1 = delay = 20
	B ASM_HOLD

ASM_DECISION_9H:
    BX LR

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 node C global

@ R0 node
@ R1 delay
ASM_HOLD:
	LDR R2, [R11, #TIME]
	ADDS R2, R2, R1
	STR R2, [R0, #NEXTTIME]			@ node->NEXTTIME = shared_state->TIME + delay

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 node C global

@ R0 node C

@ Runtime:
@ R0 node C
@ R1 = P (search pointer, starts at the tail)
@ R2 = C->NEXTTIME
ASM_SORTIN:
	LDR R1, [R11, #WAIT_LIST]		@ R1 = shared_state->WAIT_LIST.head
	LDR R1, [R1, #LEFT1]			@ R1 = P = head->left1 (last node)
	LDR R2, [R0, #NEXTTIME]			@ R2 = C->NEXTTIME
	LDR R3, [R1, #NEXTTIME]			@ R3 = P->NEXTTIME
	CMP R2, R3
	BHS ASM_SORTIN_DONE				@ unsigned: C->NEXTTIME >= P->NEXTTIME -> stop

.balign 4
ASM_SORTIN_LOOP:
	LDR R1, [R1, #LEFT1]			@ P = P->left1
	LDR R3, [R1, #NEXTTIME]			@ R3 = P->NEXTTIME
	CMP R2, R3
	BLO ASM_SORTIN_LOOP				@ unsigned: C->NEXTTIME >= P->NEXTTIME -> stop

ASM_SORTIN_DONE:
	LDR R3, [R1, #RIGHT1]			@ R3 = Q = P->right1
	STR R3, [R0, #RIGHT1]			@ C->right1 = Q
	STR R1, [R0, #LEFT1]			@ C->left1 = P
	STR R0, [R1, #RIGHT1]			@ P->right1 = C
	STR R0, [R3, #LEFT1]			@ Q->left1 = C
	BX LR

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 user

@ R0 node C
@ R1 delay
@ R2 next_inst
ASM_HOLDC:
	STR R2, [R0, #NEXTINST]				@ node->NEXTINST = next_inst
	BL ASM_HOLD

ASM_CYCLE:
	LDR R1, [R11, #WAIT_LIST]			@ R1 = shared_state->WAIT_LIST.head
	LDR R8, [R1, #RIGHT1]				@ R8 = shared_state->WAIT_LIST.head->right1

	CMP R8, R1
	BEQ ASM_CYCLE_DONE

	@ Take the earliest node off the WAIT list
	@ ElevatorNode* C = shared_state->WAIT_LIST.head->right1;	(now in R8)

	@ Advance TIME to its NEXTTIME
	LDR R0, [R8, #NEXTTIME]
	STR R0, [R11, #TIME]				@ shared_state->TIME = C->NEXTTIME;

	@ Unlink it
	MOV R0, R8
	BL elevator_list_delete_nodew		@ elevator_list_delete_nodew(C);

	@ Call NEXTINST(shared_state, C): handler branches back to ASM_CYCLE when done
	LDR R2, [R8, #NEXTINST]

	BX R2

ASM_CYCLE_DONE:
	POP {R3-R11, PC}
