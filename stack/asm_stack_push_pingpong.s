.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_stack_push_pingpong


@ Using:
@ Loop unrolling mod 4
@ Register Hoisting
@ Modulo Variable Expansion (MVE mod 4) or Circular Register Allocation:
@ MVE mod 4: registers becomes identity after 4 cyclic permutations
@ Registers R3-R6: The Ping-Pong Matrix


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ NODE_SIZE,		8
.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4


@ Input:
@ R0 Stack
@ R1 number of iterations

@ Runtime:
@ R2 tail counter
@ R3 Top
@ R4 Avail
@ R5 Next Avail

@ Return 0 or 1

asm_stack_push_pingpong:
	PUSH {R4-R6, LR}
	CBZ R1, early_return

	LDR R3, [R0, #STACK_TOP]	@ R3 = Top
	LDR R4, [R0, #STACK_AVAIL]	@ R4 = Avail

	ANDS R6, R1, #3				@ Remainder of R1(number of iterations) mod 4
	BEQ push_pingpong_loop

handle_push_tail_loop:
	@ R3 = Top, R4 = Avail
	CBZ R4, handle_overflow0	@ R4 = avail
	LDR R5, [R4, #NODE_LINK]	@ R5 = next avail
	STR R1, [R4, #NODE_INFO]	@ P->info = iterations
	STR R3, [R4, #NODE_LINK]	@ P->link = stack->top

	MOVS R3, R4					@ R3 = stack->top = avail
	MOVS R4, R5					@ R4 = avail = next avail

	SUBS R1, R1, #1				@ decrement iterations
	SUBS R6, R6, #1				@ decrement tail counter
	BNE handle_push_tail_loop
	CBZ R1, done				@ return if there was only a tail

.balign 4
push_pingpong_loop:
	@ R3 = Top, R4 = Avail, same case as in handle_tail_loop
	CBZ R4, handle_overflow0		@ R4 = P
	LDR R5, [R4, #NODE_LINK]	@ R5 = next avail node
	STR R1, [R4, #NODE_INFO]	@ P->info = iterations
	STR R3, [R4, #NODE_LINK]	@ P->link = stack->top
	SUBS R1, R1, #1				@ decrement iterations

	@ R4 = Top, R5 = Avail
	CBZ R5, handle_overflow1
	LDR R6, [R5, #NODE_LINK]	@ R6 = next avail node
	STR R1, [R5, #NODE_INFO]	@ P->info = iterations
	STR R4, [R5, #NODE_LINK]	@ P->link = stack->top
	SUBS R1, R1, #1				@ decrement iterations

	@ R5 = Top, R6 = Avail
	CBZ R6, handle_overflow2
	LDR	R3, [R6, #NODE_LINK]	@ R3 = next avail node
	STR R1, [R6, #NODE_INFO]	@ P->info = iterations
	STR R5, [R6, #NODE_LINK]	@ P->link = stack->top
	SUBS R1, R1, #1				@ decrement iterations

	@ R6 = Top, R3 = Avail
	CBZ R3, handle_overflow3
	LDR	R4, [R3, #NODE_LINK]	@ R3 = next avail node
	STR R1, [R3, #NODE_INFO]	@ P->info = iterations
	STR R6, [R3, #NODE_LINK]	@ P->link = stack->top
	SUBS R1, R1, #1				@ decrement iterations

	@ R3 = Top, R4 = Avail now
	BNE push_pingpong_loop

done:
	STR R3, [R0, #STACK_TOP]
	STR R4, [R0, #STACK_AVAIL]

early_return:
	MOVS R0, #1					@ Return true
	POP {R4-R6, PC}

handle_overflow0: STR R3, [R0, #STACK_TOP]; STR R4, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_overflow1: STR R4, [R0, #STACK_TOP]; STR R5, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_overflow2: STR R5, [R0, #STACK_TOP]; STR R6, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_overflow3: STR R6, [R0, #STACK_TOP]; STR R3, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
