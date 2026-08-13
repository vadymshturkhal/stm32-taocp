.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text

    .global ASM_USERS_INIT
	.type ASM_USERS_INIT, %function

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


@ Input:
@ R0 Users* users
@ R1 SharedState* shared_state
@ R2 Storage_Pool* storage_pool
@ R3 uint32_t users_quantity

@ Runtime:
@ R0 Users* users, ElevatorNode* USER1
@ R1 SharedState* shared_state
@ R2 Storage_Pool* storage_pool
@ R3 uint32_t users_quantity
@ R4 Users* users

@ Output:
@ R0 uint32_t status (0 = OK, 1 = NULL arg, 2 = pool empty)
ASM_USERS_INIT:
	PUSH {R4, LR}

	@ if (users == NULL || shared_state == NULL || storage_pool == NULL) return 1;
	CMP R0, #0
	BEQ USERS_INIT_ERR1
	CMP R1, #0
	BEQ USERS_INIT_ERR1
	CMP R2, #0
	BEQ USERS_INIT_ERR1

	STR R3, [R0, #USERS_QUANTITY]	@ users->users_quantity = users_quantity;
	STR R1, [R0, #SHARED_STATE]		@ users->shared_state = shared_state;
	STR R2, [R0, #STORAGE_POOL]		@ users->storage_pool = storage_pool;
	STR R0, [R1, #USERS]			@ shared_state->users = users;

	MOVS R4, R0						@ save users

	MOVS R0, R2
	BL storage_pool_pop				
	CBZ R0, USERS_INIT_ERR2			@ if (users->USER1 == NULL) return 2;
	STR R0, [R4, #USER1]			@ users->USER1 = storage_pool_pop(storage_pool);

	LDR R1, =ASM_U1
	STR R1, [R0, #NEXTINST]			@ users->USER1->NEXTINST = ASM_U1;

	MOVS R2, #0
	STR R2, [R4, #USER_ID]			@ users->user_id = 0;

	MOVS R0, #0				@ return 0
	POP {R4, PC}

USERS_INIT_ERR1:
	MOVS R0, #1
	POP {R4, PC}

USERS_INIT_ERR2:
	MOVS R0, #2
	POP {R4, PC}

@ Input:
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
@ R11 SharedState* shared_state, already
@ R10 Elevator* elevator, already
@ R9 Users* users, already
@ R8 ElevatorNode* C

@ Runtime:
@ R11 SharedState* shared_state, already
@ R10 Elevator* elevator, already
@ R9 Users* users, already
@ R8 ElevatorNode* C
ASM_U1:
	@ User fabric

	@ Not in MIX
	@ if (users->user_id >= users->users_quantity) return;
	LDR R0, [R9, #USER_ID]
	LDR R1, [R9, #USERS_QUANTITY]
	CMP R0, R1
	BHS DONE				@ Exit point

	@ 4. increment user_id (not in Coroutine U)
	ADDS R0, R0, #1
	STR R0, [R9, #USER_ID]

	@ 1. JMP VALUES
	SUB SP, SP, #16			@ Sub 16 bytes from SP for Values
	MOV R0, SP
	BL values

	@ R0 is user_values now
	MOVS R5, R0				@ Save user_values

	@ 3. Create User
	LDR R0, [R9, #STORAGE_POOL]
	BL storage_pool_pop		@ ElevatorNode* user = storage_pool_pop(users->storage_pool);
	CBZ R0, DONE_SP			@ if (user == NULL) return;

	@ Unpack Values
	LDR R1, [SP, #VALUES_IN]
	LDR R4, [SP, #VALUES_OUT]
	LDR R3, [SP, #VALUES_GIVEUPTIME]
	LDR R2, [SP, #VALUES_INTERTIME]

	STR R1, [R0, #IN]
	STR R4, [R0, #OUT]
	STR R3, [R0, #GIVEUPTIME]

	@ 2. LDA INTERTIME (time before another user enters) / JMP HOLD
	MOV R0, R11				@ Move shared_state to R0
	MOVS R1, R8				@ Move node C to R1
	@ MOVS R2, R3			@ R2 is already INTERTIME
	BL hold					@ hold(shared_state, C, user_values.INTERTIME);

	@ At the end of ASM_U1 restore Stack Pointer
	ADD SP, SP, #16

	@ User R0 goes to ASM_U2

ASM_U2:


DONE:
	B ASM_CYCLE

DONE_SP:
	ADDS SP, SP, #16
	B DONE
