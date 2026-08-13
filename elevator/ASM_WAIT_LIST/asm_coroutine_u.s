.syntax unified
    .thumb
    .cpu cortex-m4
    .section .text

    .global ASM_USERS_INIT
	.type ASM_USERS_INIT, %function

    .global ASM_USERS_START
	.type ASM_USERS_START, %function

    .global ASM_U1
	.type ASM_U1, %function


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

@ CALLS bit flags (elevator_settings.h)
.equ CALLUP,			0b100
.equ CALLDOWN,			0b010


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
	CMP R0, #0
	BEQ DONE_SP				@ if (user == NULL) return;

	MOVS R7, R0				@ Save user

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

	@ User R7 goes to ASM_U2

@ [Signal and wait]
@ Input: from ASM_U1
@ R11 SharedState* shared_state, already
@ R10 Elevator* elevator, already
@ R9 Users* users, already
@ R8 ElevatorNode* C FIXME: can replace?
@ R7 User

@ Runtime:
@ R4 ELEV1
ASM_U2:
	LDR R1, [R10, #FLOOR]	@ FIXME
	LDR R2, [R7, #IN]		@ FIXME
	SUBS R1, R1, R2
	CBNZ R1, ASM_U2_2H		@ If (elevator->FLOOR != user->IN)

	@ Is elevator positioned at E6?
	LDR R0, [R10, #ELEV1]	@ R0 = ELEV1
	LDR R1, [R0, #NEXTINST]	@ R1 = ELEV1->NEXTINST
	LDR R2, =ASM_E6
	MOVS R4, R0				@ Save ELEV1
	CMP R1, R2
	BNE ASM_U2_3H

	@ If so, reposition it at E3
	LDR R2, =ASM_E3
	STR R2, [R0, #NEXTINST]

	@ Remove it from WAIT list
	@ ELEV1 is already at R0
	BL elevator_list_delete_nodew	@ elevator_list_delete_nodew(elevator->ELEV1);

	@ And reinsert it at front of WAIT
	B ASM_U2_4H

ASM_U2_3H:
	@ Jump if D3 == 0
	LDR R0, [R10, #D3]				@ LDA D3
	CBZ R0, ASM_U2_2H				@ JAZ 2F

	@ Otherwise make D1 nonzero and set D3 = 0
	MOVS R1, #1
	MOVS R3, #0
	STR R1, [R10, #D1]
	STR R3, [R10, #D3]

ASM_U2_4H:
	@ void immed(SharedState* shared_state, ElevatorNode* wait_node)
	MOV R0, R11
	MOV R1, R4
	BL immed
	B ASM_U3

ASM_U2_2H:
	ADD R3, R11, #CALLS			@ R3 = &shared_state->CALLS[0]
	LDR R1, [R7, #IN]			@ R1 = user->IN
	LDR R2, [R7, #OUT]			@ R2 = user->OUT
	LDR R4, [R3, R1, LSL #2]	@ R4 = shared_state->CALLS[IN]
	SUBS R2, R2, R1
	BGT ASM_U2_2H_SET_CALLUP	@ J2P *+3

	@ Set CALLDOWN[IN] = 1
	ORRS R4, R4, #CALLDOWN
	STR R4, [R3, R1, LSL #2]	@ shared_state->CALLS[IN] = ...
	@ J2P *+2
	B ASM_U2_2H_CONTINUE

@ Set CALLUP[IN] = 1
ASM_U2_2H_SET_CALLUP:
	ORRS R4, R4, #CALLUP
	STR R4, [R3, R1, LSL #2]	@ shared_state->CALLS[IN] = ...

@ 5. If D2 == 0 or the elevator in its "dormant" position E1, DECISION is performed
ASM_U2_2H_CONTINUE:
	LDR R0, [R10, #D2]
	CBZ R0, ASM_U2_2H_DECISION	@ JAZ *+3

	LDR R0, [R10, #ELEV1]
	LDR R2, =ASM_E1
	LDR R1, [R0, #NEXTINST]
	SUBS R0, R1, R2
	CBZ R0, ASM_U2_2H_DECISION	@ JAZ DECISION

	B ASM_U3

ASM_U2_2H_DECISION:
	MOV R0, R11
	LDR R1, =ASM_U2
	BL decision					@ decision(shared_state, caller=U2);

@ [Enter queue]
@ Input: from ASM_U1
@ R11 SharedState* shared_state, already
@ R10 Elevator* elevator, already
@ R9 Users* users, already
@ R8 ElevatorNode* C FIXME: can replace?
@ R7 User
ASM_U3:
	@ elevator_list_insert_node_at_rear(&shared_state->QUEUE[user->IN], user);
	@ &shared_state->QUEUE[user->IN], user = shared_state + #QUEUE + IN*8

	@ Insert node at right end of QUEUE[IN]
	LDR R1, [R7, #IN]						@ R1 = user->IN
	ADD R2, R11, #QUEUE						@ R2 = shared_state + #QUEUE
	LSLS R1, R1, #3							@ R1 = IN * sizeof(ElevatorList)
	ADD R0, R1, R2							@ R0 = &shared_state->QUEUE[IN] = shared_state + #QUEUE + IN*8
	
	@ R0 = &shared_state->QUEUE[IN]
	MOVS R1, R7								@ R1 = user
	BL elevator_list_insert_node_at_rear	@ elevator_list_insert_node_at_rear(&QUEUE[IN], user);

ASM_U4A:
	@ FIXME

DONE:
	B ASM_CYCLE

DONE_SP:
	ADDS SP, SP, #16
	B DONE
