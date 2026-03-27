.syntax unified
    .thumb
    .cpu cortex-m4
    .global asm_stack_pop_pingpong


@ Using:
@ Loop unrolling mod 4
@ Register Hoisting
@ Modulo Variable Expansion (MVE mod 4) or Circular Register Allocation:
@ MVE mod 4: registers permutation becomes identity after 4 cyclic permutations


@ Memory offsets
.equ NODE_INFO, 	0
.equ NODE_LINK, 	4
.equ STACK_TOP, 	0
.equ STACK_AVAIL,	4


@ Input:
@ R0 Stack
@ R1 number of iterations

@ Runtime:
@ R2 tail counter, info
@ R3 Avail
@ R4 Top
@ R5 Next Avail

@ Return 0 or 1

asm_stack_pop_pingpong:
	PUSH {R4-R6, LR}
	CBZ R1, early_return

	LDR R3, [R0, #STACK_AVAIL]	@ R3 = Avail
	LDR R4, [R0, #STACK_TOP]	@ R4 = Top

	ANDS R6, R1, #3				@ Remainder of R1(number of iterations) mod 4
	BEQ pop_pingpong_loop

handle_pop_tail_loop:
	@ R4 = Top, R3 = Avail
	CBZ R4, handle_underflow0	@ R4 = Top
	LDR R5, [R4, #NODE_LINK]	@ R5 = next top
	LDR R2, [R4, #NODE_INFO]	@ R2 = info = top->info;
	STR R3, [R4, #NODE_LINK]	@ P->link = stack->avail;

	MOVS R3, R4					@ R3 = avail = top
	MOVS R4, R5					@ R4 = top = next top

	SUBS R1, R1, #1				@ decrement iterations
	SUBS R6, R6, #1				@ decrement tail counter
	BNE handle_pop_tail_loop
	CBZ R1, done				@ return if there was only a tail

.balign 4
pop_pingpong_loop:
	@ R4 = Top, R3 = Avail
	CBZ R4, handle_underflow0
	LDR R5, [R4, #NODE_LINK]	@ R5 = next top
	LDR R2, [R4, #NODE_INFO]	@ R2 = info = top->info;
	STR R3, [R4, #NODE_LINK]	@ P->link = stack->avail;
	SUBS R1, R1, #1				@ decrement iterations

	@ R5 = Top, R4 = Avail
	CBZ R5, handle_underflow1
	LDR R6, [R5, #NODE_LINK]	@ R5 = next top
	LDR R2, [R5, #NODE_INFO]	@ R2 = info = top->info;
	STR R4, [R5, #NODE_LINK]	@ P->link = stack->avail;
	SUBS R1, R1, #1				@ decrement iterations

	@ R6 = Top, R5 = Avail
	CBZ R6, handle_underflow2
	LDR R3, [R6, #NODE_LINK]	@ R5 = next top
	LDR R2, [R6, #NODE_INFO]	@ R2 = info = top->info;
	STR R5, [R6, #NODE_LINK]	@ P->link = stack->avail;
	SUBS R1, R1, #1				@ decrement iterations

	@ R3 = Top, R6 = Avail
	CBZ R3, handle_underflow3
	LDR	R4, [R3, #NODE_LINK]	@ R3 = next avail node
	LDR R2, [R3, #NODE_INFO]	@ P->info = iterations
	STR R6, [R3, #NODE_LINK]	@ P->link = stack->top
	SUBS R1, R1, #1				@ decrement iterations

	@ R4 = Top, R3 = Avail now
	BNE pop_pingpong_loop

done:
	STR R3, [R0, #STACK_AVAIL]
	STR R4, [R0, #STACK_TOP]

early_return:
	MOVS R0, #1
	POP {R4-R6, PC}

handle_underflow0: STR R4, [R0, #STACK_TOP]; STR R3, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_underflow1: STR R5, [R0, #STACK_TOP]; STR R4, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_underflow2: STR R6, [R0, #STACK_TOP]; STR R5, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
handle_underflow3: STR R3, [R0, #STACK_TOP]; STR R6, [R0, #STACK_AVAIL]; MOVS R0, #0; POP {R4-R6, PC}
