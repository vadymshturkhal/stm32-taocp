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

	.global ASM_E6B
	.type ASM_E6B, %function

	.global ASM_E7
	.type ASM_E7, %function

	.global ASM_E7_CONTINUE
	.type ASM_E7_CONTINUE, %function

	.global ASM_E8
	.type ASM_E8, %function

	.global ASM_E8_CONTINUE
	.type ASM_E8_CONTINUE, %function

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

@ SharedState fields definition
.equ TIME, 				0
.equ CALLS,				4
.equ ELEVATOR_LIST, 	24
.equ QUEUE,				32
.equ WAIT_LIST,	 		72
.equ ELEVATOR,			80
.equ USERS,				84

@ FLOORS
.equ HOME_FLOOR,		2
.equ FLOORS,			5

@ CALLS bit flags (elevator_settings.h)
.equ CALLUP,			0b100
.equ CALLDOWN,			0b010
.equ CALLCAR,			0b001

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
	MOVS R0, R8					@ R0 node C
	MOVS R1, R7					@ R1 delay
	LDR R2, =ASM_E2				@ R2 next_inst
	B ASM_HOLDC					@ ASM_CYCLE is automatically invoked after ASM_HOLDC

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

@ NOTE: magic 4 is sizeof CALLS, which is uint32_t
ASM_E2_HIGHER_CALLS:
	LDR R1, [R10, #FLOOR]

	ADD R5, R11, #CALLS							@ R5 = &CALLS[0]
	ADD R3, R5, R1, LSL #2						@ R3 = &CALLS[FLOOR] = Current floor CALLS
	ADD R4, R11, #(CALLS + (FLOORS - 1) * 4)	@ R4 = &CALLS[FLOORS - 1] = Higher floor CALLS

	CMP R4, R3
	BLE ASM_E2_ELEVATOR_LOWER_FLOORS

@ NOTE: MIX has 4 additional "paddings" for simply adding calls, without any loop
.balign 4
ASM_E2_HIGHER_LOOP:
	LDR R0, [R4], #-4					@ sizeof CALLS is uint32_t

	@ So long distance for CBNZ
	CMP R0, #0
	BGT ASM_E3							@ Early jump to ASM_E3 if found higher call

	CMP R4, R3
	BNE ASM_E2_HIGHER_LOOP

@ Have passengers in the elevator called for lower floors?
@ Runtime:
@ R3 = &CALLS[FLOOR]
@ R5 = &CALLS[0]
ASM_E2_ELEVATOR_LOWER_FLOORS:
	CMP R5, R3
	BGE ASM_E2_LOWER_DONE				@ FLOOR == 0: no lower floors exist

ASM_E2_LOWER_LOOP:
	LDR R0, [R5], #4					@ sizeof CALLS is uint32_t
	ANDS R0, R0, #CALLCAR

	CBNZ R0, ASM_E2_2H					@ Early jump to ASM_E2_2H if found lower call

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

	@ CBNZ R0, ASM_E3						@ Early jump to ASM_E3 if found lower call
	CMP R0, #0
	BNE ASM_E3

	CMP R5, R3
	BNE ASM_E2_LOWER_CALLCAR_LOOP

@ Have passengers in the elevator called for higher floors?
@ Runtime:
@ R3 &CALLS[FLOOR] = Current floor CALLS
@ R4 &CALLS[FLOORS - 1]

@ NOTE: magic 4 is sizeof CALLS, which is uint32_t
ASM_E2_ELEVATOR_HIGHER_FLOORS:
	ADD R4, R11, #(CALLS + (FLOORS - 1) * 4)	@ R4 = &CALLS[FLOORS - 1] = Higher floor CALLS
	MOVS R0, #0									@ R0 = rA = Accumulator

	CMP R4, R3
	BLE ASM_E2_2H

ASM_E2_HIGHER_CALLCAR_LOOP:
	LDR R0, [R4], #-4					@ sizeof CALLS is uint32_t
	ANDS R0, R0, #CALLCAR

	CBNZ R0, ASM_E2_2H					@ Early jump to ASM_E2_2H if found higher call

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
	@ CBNZ R0, ASM_E3
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
	MOVS R0, R4				@ R0 node
	MOV R1, #300			@ R1 delay
	BL ASM_HOLD

	LDR R0, [R10, #ELEV2]	@ R0 node
	MOV R1, #76				@ R1 delay
	BL ASM_HOLD

	MOVS R0, #1
	STR R0, [R10, #D2]		@ elevator->D2 = 1;
	STR R0, [R10, #D1]		@ elevator->D1 = 1;

	MOV R7, #20				@ delay = 20

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ R7 uint32_t delay
ASM_E4A:
	LDR R2, =ASM_E4
	LDR R0, [R10, #ELEV1]
	MOV R1, R7
	B ASM_HOLDC

@ [Let people out, in]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R4 = ELEVATOR_LIST.head (search sentinel), later &QUEUE[FLOOR] / QUEUE[FLOOR].head
@ R5 = node (current search pointer)
@ R6 = elevator->FLOOR
ASM_E4:
	LDR R4, [R11, #ELEVATOR_LIST]			@ R4 = shared_state->ELEVATOR_LIST.head
	MOVS R5, R4
	LDR R6, [R10, #FLOOR]					@ R6 = elevator->FLOOR

@ [Search ELEVATOR list, right to left]
@ NOTE: MIX has 4 additional "paddings" for simply adding calls, without any loop
ASM_E4_ELEVATOR_LOOP:
	LDR R5, [R5, #LEFT2]					@ R5 = node = head->left2
	CMP R5, R4
	BEQ ASM_E4_1H							@ node == head: search complete

	@ Compare user->OUT with FLOOR
	LDR R0, [R5, #OUT]
	CMP R0, R6
	BNE ASM_E4_ELEVATOR_LOOP				@ If not equal: continue searching

	LDR R1, =ASM_U6							@ Otherwise prepare to send user to U6
	B ASM_E4_2H

ASM_E4_1H:
	ADD R4, R11, #QUEUE						@ R4 = &QUEUE[0]
	ADD R4, R4, R6, LSL #3					@ R4 = &QUEUE[FLOOR] (sizeof(ElevatorList) == 8)
	LDR R4, [R4]							@ R4 = QUEUE[FLOOR].head
	LDR R5, [R4, #RIGHT2]					@ R5 = node = QUEUE[FLOOR].head->right2

	CMP R5, R4
	BEQ ASM_E4_1H_QUEUE_EMPTY				@ node == head: QUEUE is empty

	MOVS R0, R5								@ If not, cancel action U4 for this user
	BL elevator_list_delete_nodew			@ elevator_list_delete_nodew(node)

	LDR R1, =ASM_U5							@ Prepare to replace U4 by U5

ASM_E4_2H:
	STR R1, [R5, #NEXTINST]					@ node->NEXTINST = ASM_U5

	MOVS R0, R11
	MOVS R1, R5
	BL immed								@ immed(shared_state, node)

	MOV R7, #25								@ delay = 25
	B ASM_E4A								@ E4A(shared_state, delay)

ASM_E4_1H_QUEUE_EMPTY:
	MOVS R0, #0
	STR R0, [R10, #D1]						@ elevator->D1 = 0

	MOVS R0, #1
	STR R0, [R10, #D3]						@ elevator->D3 = 1

	B ASM_CYCLE

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ R7 uint32_t delay
ASM_E5A:
	LDR R2, =ASM_E5
	LDR R0, [R10, #ELEV2]
	MOV R1, R7
	B ASM_HOLDC

@ [Close door]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E5:
	LDR R0, [R10, #D1]
	CBZ R0, ASM_E5_CLOSE				@ D1 == 0: proceed to close

	MOV R7, #40							@ delay = 40
	B ASM_E5A							@ Elevator doors flutter

ASM_E5_CLOSE:
	MOVS R0, #0
	STR R0, [R10, #D3]					@ elevator->D3 = 0;

	LDR R2, =ASM_E6
	LDR R0, [R10, #ELEV1]
	MOVS R1, #20
	B ASM_HOLDC

@ [Prepare to move]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E6:
	@ If STATE != GOINGDOWN: CALLUP and CALLCAR on this floor are reset
	LDR R0, [R10, #STATE]
	CMP R0, #0
	BLT ASM_E6_SKIP_UP_RESET

	LDR R1, [R10, #FLOOR]
	ADD R2, R11, #CALLS
	LDR R3, [R2, R1, LSL #2]
	BIC R3, R3, #(CALLUP | CALLCAR)
	STR R3, [R2, R1, LSL #2]

ASM_E6_SKIP_UP_RESET:
	@ If STATE != GOINGUP: reset CALLCAR and CALLDOWN
	@ LDR R0, [R10, #STATE]
	@ CMP R0, #0
	BGT ASM_E6_SKIP_DOWN_RESET				@ flag preserved from ASM_E6

	LDR R1, [R10, #FLOOR]
	ADD R2, R11, #CALLS
	LDR R3, [R2, R1, LSL #2]
	BIC R3, R3, #(CALLCAR | CALLDOWN)
	STR R3, [R2, R1, LSL #2]

ASM_E6_SKIP_DOWN_RESET:
	@ LDR R0, [R10, #STATE]
	CMP R0, #0
	BNE ASM_E6B
	@ CBNZ R0, ASM_E6B						@ flag preserved from ASM_E6

	BL ASM_DECISION

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R4 STATE
ASM_E6B:
	LDR R4, [R10, #STATE]
	CBNZ R4, ASM_E6B_D2_CHECK

	B ASM_E1A							@ If STATE == NEUTRAL, go to E1 and wait

ASM_E6B_D2_CHECK:
	LDR R0, [R10, #D2]
	CBZ R0, ASM_E6B_SCHEDULE

	LDR R0, [R10, #ELEV3]
	BL elevator_list_delete_nodew		@ Cancel activity E9

	LDR R0, [R10, #ELEV3]
	MOVS R1, #0
	STR R1, [R0, #LEFT1]				@ elevator->ELEV3->left1 = NULL;

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
@ R4 STATE
ASM_E6B_SCHEDULE:
	LDR R2, =ASM_E7
	LDR R0, [R10, #ELEV1]
	MOVS R1, #15

	CMP R4, #0
	BGE ASM_E7A							@ STATE >= 0: go to E7 (already loaded)

	LDR R2, =ASM_E8
	B ASM_E8A							@ STATE == GOINGDOWN: go to E8

ASM_E7A:
	B ASM_HOLDC

@ [Go up a floor]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E7:
	LDR R0, [R10, #FLOOR]
	ADDS R0, R0, #1
	STR R0, [R10, #FLOOR]					@ elevator->FLOOR += 1;

	LDR R2, =ASM_E7_CONTINUE
	LDR R0, [R10, #ELEV1]
	MOVS R1, #51
	B ASM_HOLDC

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R1 = elevator->FLOOR
@ R0 = CALLS[FLOOR]
@ R2 = &CALLS[0]
ASM_E7_CONTINUE:
	@ Is CALLCAR[FLOOR] or CALLUP[FLOOR] != 0?
	LDR R1, [R10, #FLOOR]
	ADD R2, R11, #CALLS
	LDR R0, [R2, R1, LSL #2]				@ R0 = CALLS[FLOOR]
	TST R0, #(CALLCAR | CALLUP)
	BNE ASM_E7_1H							@ If yes: it is time to stop elevator

    @ If not, is FLOOR == 2?
	CMP R1, #HOME_FLOOR
	BEQ ASM_E7_2H_HIGHER_LOOP

	@ If not, is CALLDOWN[FLOOR] != 0
	TST R0, #CALLDOWN
	BEQ ASM_E7								@ If not, repeat step E7

@ Runtime:
@ R3 = &CALLS[FLOOR]
@ R4 = &CALLS[FLOORS - 1]

@ NOTE: magic 4 is sizeof CALLS, which is uint32_t
ASM_E7_2H_HIGHER_LOOP:
	ADD R3, R2, R1, LSL #2						@ R3 = &CALLS[FLOOR]
	ADD R4, R11, #(CALLS + (FLOORS - 1) * 4)	@ R4 = &CALLS[FLOORS - 1]

	CMP R4, R3
	BLE ASM_E7_1H								@ No higher calls

@ NOTE: MIX has 4 additional "paddings" for simply adding calls, without any loop
ASM_E7_2H_CONTINUE_HIGHER_LOOP:
	LDR R0, [R4], #-4
	CMP R0, #0
	BGT ASM_E7								@ found higher call: repeat E7; return;

	CMP R4, R3
	BNE ASM_E7_2H_CONTINUE_HIGHER_LOOP

	@ No higher call found

@ Runtime:
@ R7 uint32_t delay
ASM_E7_1H:
	MOV R7, #14
	B ASM_E2A								@ It's time to stop elevator: E2A(shared_state, elevator->ELEV1, 14);

@ [Go down a floor]
ASM_E8A:
	B ASM_HOLDC

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E8:
	LDR R0, [R10, #FLOOR]
	SUBS R0, R0, #1
	STR R0, [R10, #FLOOR]					@ elevator->FLOOR -= 1;

	LDR R2, =ASM_E8_CONTINUE
	LDR R0, [R10, #ELEV1]
	MOVS R1, #61
	B ASM_HOLDC

@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C

@ Runtime:
@ R1 = elevator->FLOOR
@ R0 = CALLS[FLOOR] (until clobbered by the search loop)
@ R2 = &CALLS[0]
ASM_E8_CONTINUE:
	@ Is CALLCAR[FLOOR] or CALLDOWN[FLOOR] != 0?
	LDR R1, [R10, #FLOOR]
	ADD R2, R11, #CALLS
	LDR R0, [R2, R1, LSL #2]				@ R0 = CALLS[FLOOR]
	TST R0, #(CALLCAR | CALLDOWN)
	BNE ASM_E8_1H							@ If yes: it is time to stop elevator

	@ If not, is FLOOR == HOME_FLOOR?
	CMP R1, #HOME_FLOOR
	BEQ ASM_E8_2H_LOWER_LOOP

	@ If not, is CALLUP[FLOOR] != 0
	TST R0, #CALLUP
	BEQ ASM_E8								@ If not, repeat step E8

@ Runtime:
@ R3 = &CALLS[FLOOR]
@ R4 = &CALLS[j], from 0 to FLOOR

@ NOTE: magic 4 is sizeof CALLS, which is uint32_t
ASM_E8_2H_LOWER_LOOP:
	ADD R3, R2, R1, LSL #2					@ R3 = &CALLS[FLOOR]
	MOVS R4, R2								@ R4 = &CALLS[0]

	CMP R4, R3
	BGE ASM_E8_1H							@ No lower calls

ASM_E8_2H_CONTINUE_LOWER_LOOP:
	LDR R0, [R4], #4						@ sizeof CALLS is uint32_t
	CMP R0, #0
	BGT ASM_E8								@ found lower call: repeat E8; return;

	CMP R4, R3
	BNE ASM_E8_2H_CONTINUE_LOWER_LOOP

	@ No lower call found

@ Runtime:
@ R7 uint32_t delay
ASM_E8_1H:
	MOV R7, #23
	B ASM_E2A								@ It's time to stop elevator: E2A(shared_state, elevator->ELEV1, 23);

@ [Set inaction indicator]
@ Input:
@ R11 SharedState* shared_state
@ R10 Elevator* elevator
@ R9 Users* users
@ R8 ElevatorNode* C
ASM_E9:
	LDR R0, [R10, #ELEV3]
	MOVS R1, #0
	STR R1, [R0, #LEFT1]					@ elevator->ELEV3->left1 = NULL; STZ 0,6
	STR R1, [R10, #D2]						@ elevator->D2 = 0;	STZ D2

	BL ASM_DECISION
	B ASM_CYCLE
