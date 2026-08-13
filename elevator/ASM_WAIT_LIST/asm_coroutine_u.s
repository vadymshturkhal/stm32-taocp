.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text

    .global ASM_USERS_START
	.type ASM_USERS_START, %function

    .global ASM_COROUTINE_U
	.type ASM_COROUTINE_U, %function

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

@ Users fields definition
@ .equ SHARED_STATE, 		0
.equ STORAGE_POOL,		4
.equ USER1, 			8
.equ USER_ID,			12
.equ USERS_QUANTITY,	16

@ Values fields definition
.equ VALUES_IN, 		0
.equ VALUES_OUT,		4
.equ VALUES_GIVEUPTIME, 8
.equ VALUES_INTERTIME,	12


@ R0 SharedState* shared_state
ASM_USERS_START:
	PUSH {LR}						@ non-leaf: must preserve caller's return address across the BL below

	@ USER1 node represents action U1 and it is initially the sole entry in the WAIT list
	@ immed(shared_state, shared_state->users->USER1);
	LDR R1, [R0, #USERS]
	LDR R1, [R1, #USER1]
	BL immed

	POP {PC}						@ restore original LR

@ [Enter, prepare for successor]
@ Input:
@ R0 SharedState* shared_state
@ R1 ElevatorNode* C

@ Runtime:
@ R11 SharedState* shared_state, already
@ R10 Elevator* elevator, already
@ R9 Users* users, already
ASM_U1:
	PUSH {R4-R6, LR}
	// User fabric

	@ Users* users = shared_state->users;

	// Not in MIX -- Python stops once its fixed user list is exhausted;
	// this port stops once users_quantity users have been generated
	@ if (users->user_id >= users->users_quantity) return;
	LDR R0, [R9, #USER_ID]
	LDR R2, [R9, #USERS_QUANTITY]
	CMP R0, R2
	BHS DONE				@ Exit point

	// 4. increment user_id (not in Coroutine U)
	ADDS R0, R0, #1
	STR R0, [R9, #USER_ID]

	MOVS R4, R1				@ Save node C

	SUB SP, SP, #16			@ Sub 16 bytes from SP for Values
	MOV R0, SP

	@ 1. JMP VALUES
	BL values

	@ R0 is user_values now
	MOVS R5, R0				@ Save user_values

	@ 3. Create User
	LDR R0, [R9, #STORAGE_POOL]
	BL storage_pool_pop		@ ElevatorNode* user = storage_pool_pop(users->storage_pool);
	CBZ R0, DONE_SP			@ if (user == NULL) return;

	MOVS R6, R0				@ Save user

	LDR R0, [SP, #VALUES_IN]
	LDR R1, [SP, #VALUES_OUT]
	LDR R3, [SP, #VALUES_GIVEUPTIME]
	LDR R2, [SP, #VALUES_INTERTIME]

	STR R0, [R6, #IN]
	STR R1, [R6, #OUT]
	STR R3, [R6, #GIVEUPTIME]

	@ 2. LDA INTERTIME (time before another user enters) / JMP HOLD
	MOV R0, R11
	MOVS R1, R4
	@ MOVS R2, R3			@ R2 is already INTERTIME
	BL hold					@ hold(shared_state, C, user_values.INTERTIME);

	@ At the end of ASM_U1 restore Stack Pointer
	ADDS SP, SP, #16

ASM_U2:


DONE:
	POP {R4-R6, PC}

DONE_SP:
	ADDS SP, SP, #16
	B DONE
