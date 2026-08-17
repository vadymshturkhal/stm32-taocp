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

    .global ASM_U2
	.type ASM_U2, %function

	.global ASM_U4
	.type ASM_U4, %function

	.global ASM_U5
	.type ASM_U5, %function

	.global ASM_U6
	.type ASM_U6, %function

#ifdef TRACE
	.extern trace

	.section .rodata
ASM_U1_TRACE_STEP: .asciz "U1"
ASM_U1_TRACE_FMT:  .asciz "User %u arrives at floor %u, destination is %u, give up %u"
ASM_U4_TRACE_STEP: .asciz "U4"
ASM_U4_TRACE_FMT:  .asciz "User %u decides to give up, leaves the system"
ASM_U6_TRACE_STEP: .asciz "U6"
ASM_U6_TRACE_FMT:  .asciz "User %u gets out, leaves the system"
	.section .text
#endif

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
.equ ID,				36

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
.equ CALLCAR,			0b001

@ Storage_Pool fields definition
.equ AVAIL,				0
.equ NODE_SIZE,			4
.equ POOL_SIZE,			8


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
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R7 ElevatorNode* user
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
	LDR R6, [R9, #STORAGE_POOL]		@ R6 = storage_pool
	LDR R0, [R6, #AVAIL]				@ R0 = user = storage_pool->avail
	CMP R0, #0
	BEQ DONE_SP				@ if (user == NULL) return;

	@ AVAIL POP
	LDR R1, [R0]						@ R1 = *(void**)user (node's own link field)
	STR R1, [R6, #AVAIL]				@ storage_pool->avail = R1
	@ LDR R1, [R6, #POOL_SIZE]
	@ SUBS R1, R1, #1
	@ STR R1, [R6, #POOL_SIZE]			@ storage_pool->size -= 1

	MOVS R7, R0				@ Save user

	@ Unpack Values
	LDR R1, [SP, #VALUES_IN]
	LDR R4, [SP, #VALUES_OUT]
	LDR R3, [SP, #VALUES_GIVEUPTIME]
	LDR R2, [SP, #VALUES_INTERTIME]

	STR R1, [R0, #IN]
	STR R4, [R0, #OUT]
	STR R3, [R0, #GIVEUPTIME]
#ifdef TRACE
	LDR R6, [R9, #USER_ID]			@ R6 = users->user_id (post-increment, already stored back)
	SUBS R6, R6, #1					@ R6 = id = pre-increment value
	STR R6, [R0, #ID]				@ user->id = id
#endif

	@ 2. LDA INTERTIME (time before another user enters) / JMP HOLD
	@ MOV R0, R11				@ Move shared_state to R0
	@ MOVS R1, R8				@ Move node C to R1
	@ @ MOVS R2, R3			@ R2 is already INTERTIME
	@ BL hold					@ hold(shared_state, C, user_values.INTERTIME);

	MOVS R0, R8				@ R0 node
	MOVS R1, R2				@ R1 delay = INTERTIME
	BL ASM_HOLD

#ifdef TRACE
	@ R7 still holds user (read-only below); R4/R5/R6 are dead after ASM_HOLD/SORTIN's own internal use.
	@ Reuse the still-open VALUES frame (SP+0..SP+12) for the 3 stack-spilled varargs -- its own
	@ contents were fully consumed into registers back at line ~206-209, so it's dead space here.
	LDR R4, [R7, #IN]
	STR R4, [SP, #0]					@ stack arg2 = user->IN
	LDR R4, [R7, #OUT]
	STR R4, [SP, #4]					@ stack arg3 = user->OUT
	LDR R4, [R11, #TIME]
	LDR R5, [R7, #GIVEUPTIME]
	ADDS R4, R4, R5
	STR R4, [SP, #8]					@ stack arg4 = shared_state->TIME + user->GIVEUPTIME

	LDR R3, [R7, #ID]					@ R3 = user->id (reg-passed arg1)
	MOVS R0, R11						@ R0 = shared_state
	LDR R1, =ASM_U1_TRACE_STEP
	LDR R2, =ASM_U1_TRACE_FMT

	MOV R6, R12							@ save R12 (caller-saved) across the call
	BL trace
	MOV R12, R6							@ restore R12 = WAIT_LIST.head
#endif

	@ At the end of ASM_U1 restore Stack Pointer
	ADD SP, SP, #16

	@ User R8 goes to ASM_U2
	MOVS R8, R7				@ Replace C with user

@ [Signal and wait]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user

@ Runtime:
@ R4 ELEV1
ASM_U2:
	LDR R1, [R10, #FLOOR]	@ FIXME
	LDR R2, [R8, #IN]		@ FIXME
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
	BL ASM_DELETEW

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
	LDR R1, [R8, #IN]			@ R1 = user->IN
	LDR R2, [R8, #OUT]			@ R2 = user->OUT
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
	BL ASM_DECISION

@ [Enter queue]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user
ASM_U3:
	@ elevator_list_insert_node_at_rear(&shared_state->QUEUE[user->IN], user);
	@ &shared_state->QUEUE[user->IN], user = shared_state + #QUEUE + IN*8

	@ Insert node at right end of QUEUE[IN]
	LDR R1, [R8, #IN]						@ R1 = user->IN
	ADD R2, R11, #QUEUE						@ R2 = shared_state + #QUEUE
	LSLS R1, R1, #3							@ R1 = IN * sizeof(ElevatorList)
	ADD R0, R1, R2							@ R0 = &shared_state->QUEUE[IN] = shared_state + #QUEUE + IN*8
	
	@ R0 = &shared_state->QUEUE[IN]
	MOVS R1, R8								@ R1 = user
	BL ASM_INSERT

@ [Wait GIVEUPTIME units]
ASM_U4A:
	@ LDA GIVEUPTIME
	@ JMP HOLDC
	LDR R1, [R8, #GIVEUPTIME]
	LDR R2, =ASM_U4
	MOVS R0, R8
	B ASM_HOLDC

@ [Give up]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user, also if called from ASM_CYCLE
ASM_U4:
	@ If the user's IN floor differs from the elevator's current FLOOR: give up
	LDR R0, [R8, #IN]					@ user->IN
	LDR R1, [R10, #FLOOR]				@ elevator->FLOOR
	SUBS R0, R0, R1						@ IN(C) - FLOOR

	@ NOTE: CBNZ can't be used with .global identifier
	@ CBNZ R0, ASM_U6
	CBNZ R0, ASM_U6_BEFORE

	@ CMP R0, #0
	@ BNE ASM_U6								@ JANZ *+3

	@ JANZ U4A: doors still busy: reschedule U4
	LDR R1, [R10, #D1]					@ R1 = D1
	CMP R1, #0
	BNE ASM_U4A							@ JANZ U4A

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user, also if called from ASM_CYCLE
ASM_U6_BEFORE:
#ifdef TRACE
	MOVS R0, R11
	LDR R1, =ASM_U4_TRACE_STEP
	LDR R2, =ASM_U4_TRACE_FMT
	LDR R3, [R8, #ID]
	MOV R6, R12							@ save R12 (caller-saved) across the call
	BL trace
	MOV R12, R6							@ restore R12 = WAIT_LIST.head
	B ASM_U6_BODY							@ give-up path skips U6's own trace
#endif

@ [Get out]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user, also if called from ASM_CYCLE
ASM_U6:
#ifdef TRACE
	MOVS R0, R11
	LDR R1, =ASM_U6_TRACE_STEP
	LDR R2, =ASM_U6_TRACE_FMT
	LDR R3, [R8, #ID]
	MOV R6, R12							@ save R12 (caller-saved) across the call
	BL trace
	MOV R12, R6							@ restore R12 = WAIT_LIST.head
#endif

ASM_U6_BODY:
	MOVS R0, R8
	BL ASM_DELETE

	@ AVAIL PUSH
	LDR R0, [R9, #STORAGE_POOL]			@ R0 = shared_state->users->storage_pool
	LDR R2, [R0, #AVAIL]					@ R2 = storage_pool->avail
	STR R2, [R8]							@ *(void**)user = storage_pool->avail (node's own link field)
	STR R8, [R0, #AVAIL]					@ storage_pool->avail = user
	@ LDR R2, [R0, #POOL_SIZE]
	@ ADDS R2, R2, #1
	@ STR R2, [R0, #POOL_SIZE]				@ storage_pool->size += 1

	@ FIXME
	LDR R8, [R12, #RIGHT1]				@ R8 = shared_state->WAIT_LIST.head->right1
	B ASM_CYCLE

@ [Get in]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* user, also if called from ASM_CYCLE
@ Runtime:
@ R7 uint32_t delay
ASM_U5:
	@ 1. This user now leaves QUEUE and enters ELEVATOR
	
	@ Delete User from QUEUE[IN]
	MOVS R0, R8
	BL ASM_DELETE

	@ Insert it at right of ELEVATOR
	ADD R0, R11, #ELEVATOR_LIST				@ R0 = &shared_state->ELEVATOR_LIST
	MOVS R1, R8
	BL ASM_INSERT

	@ 2. Set CALLCAR[OUT] = 1
	LDR R0, [R8, #OUT]						@ R0 = user->OUT
	ADD R1, R11, #CALLS						@ R1 = &shared_state->CALLS[0]
	LDR R2, [R1, R0, LSL #2]				@ R2 = shared_state->CALLS[OUT]
	ORRS R2, R2, #CALLCAR
	STR R2, [R1, R0, LSL #2]

	@ 3. If STATE == NEUTRAL, set STATE = GOINGUP or GOINGDOWN as appropriate
	LDR R0, [R10, #STATE]
	CMP R0, #0
	IT NE
	LDRNE R8, [R12, #RIGHT1]			@ R8 = shared_state->WAIT_LIST.head->right1 (only if leaving via ASM_CYCLE)
	BNE ASM_CYCLE

	LDR R0, [R10, #ELEV2]					@ R0 = elevator->ELEV2

	LDR R2, [R8, #OUT]						@ R2 = user->OUT
	LDR R1, [R10, #FLOOR]					@ R1 = elevator->FLOOR
	SUBS R2, R2, R1							@ R2 = OUT - FLOOR
	STR R2, [R10, #STATE]					@ elevator->STATE = OUT - FLOOR

	@ 4. Remove action E5 from WAIT list
	BL ASM_DELETEW

	@ Restart E5 after 25 units
	MOVS R7, #25
	BL ASM_E5A

DONE:
	@ FIXME
	LDR R8, [R12, #RIGHT1]				@ R8 = shared_state->WAIT_LIST.head->right1
	B ASM_CYCLE

DONE_SP:
	ADDS SP, SP, #16
	B DONE
