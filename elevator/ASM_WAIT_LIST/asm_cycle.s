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

@ NOTE: R8-R11 are global registers which contain global state and don't need to PUSH them every step
@ NOTE: Using C NextInst with global parameters permanently stored in R8-R11 Registers
@ NOTE: Helper subroutines (e.g. HOLDC) is written in C
@ FIXME: Rewrite DECISION to use LR as a caller

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
.equ ELEVATOR_LIST_HEAD, 0
@.equ ELEVATOR_LIST_STORAGE_POOL,4

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

@ Input:
@ R0 SharedState* shared_state

@ Runtime: Set global variables
@ R0-R3 scratch
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_CYCLE_START:
	PUSH {R4-R11, LR}			@ Save all Registers
	MOVS R11, R0				@ R11 = shared_state
	LDR R10, [R0, #ELEVATOR]	@ R10 = elevator
	LDR R9, [R0, #USERS]		@ R9 = users

.balign 4
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
	
	@ FIXME: remove comments
	@ MOV R0, R11							@ R0 = shared_state
	@ MOV R1, R8							@ R1 = C
	BX R2

ASM_CYCLE_DONE:
	POP {R4-R11, PC}
