#include <stdint.h>
#include <stdbool.h>
#ifdef TRACE
#include <stdio.h>
#endif

#include "main.h"

// Prototypes
uint32_t start_elevator_simulation(uint32_t max_users);
uint32_t start_asm_elevator_simulation(uint32_t max_users);

uint32_t measure_elevator_performance() {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint32_t max_users = 66;

#ifdef TRACE
	printf("TIME STATE FLOOR D1 D2 D3 step action\r\n");
#endif
	// 66 users
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 90322 | warm cycles = 90379
	// GCC -O3 -mcpu=cortex-m4 -mthumb -flto: cold cycles = 77794 | warm cycles = 77782
	// Flash size = ?
	start = DWT->CYCCNT;
	uint32_t gcc_elevator_status = start_elevator_simulation(max_users);
	if (gcc_elevator_status != 0) return gcc_elevator_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_elevator_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_elevator_status = start_elevator_simulation(max_users);
	if (gcc_elevator_status != 0) return gcc_elevator_status;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_elevator_cycles_warm = (end - start) - overhead;

#ifdef TRACE
	printf("TIME STATE FLOOR D1 D2 D3 step action\r\n");
#endif
	// 66 users
	// ASM: cold cycles = 88401 | warm cycles = 88295
	// ASM: cold cycles = 86343 | warm cycles = 86230
	// ASM: cold cycles = 86307 | warm cycles = 86316
	// ASM: cold cycles = 85488 | warm cycles = 85223
	// ASM: cold cycles = 84664 | warm cycles = 84665
	// ASM: cold cycles = 83980 | warm cycles = 83977
	// ASM: cold cycles = 83755 | warm cycles = 83674
	// ASM: cold cycles = 83571 | warm cycles = 83605
	// ASM: cold cycles = 83211 | warm cycles = 83128
	// ASM: cold cycles = 83070 | warm cycles = 82992
	// ASM: cold cycles = 83122 | warm cycles = 82973
	// ASM: cold cycles = 82558 | warm cycles = 82554
	// ASM: cold cycles = 82606 | warm cycles = 82481
	// ASM: cold cycles = 82528 | warm cycles = 82485
	// ASM: cold cycles = 82680 | warm cycles = 82433
	// ASM: cold cycles = 82504 | warm cycles = 82379				After ASM_DELETEW
	// ASM: cold cycles = 82030 | warm cycles = 81974				After ASM_DELETE
	// ASM: cold cycles = 82776 | warm cycles = 82645				After ASM_INSERT creation
	// ASM: cold cycles = 82117 | warm cycles = 82074				After ASM_INSERT using
	// ASM: cold cycles = 80692 | warm cycles = 80672				After AVAIL PUSH/POP inlining
	// ASM: cold cycles = 79129 | warm cycles = 79111				After removing size updating from AVAIL PUSH/POP inlining
	// ASM: cold cycles = 77397-77772 | warm cycles = 77382-77498	After adding ASM_IMMED
	// ASM: cold cycles = 76829 | warm cycles = 76929				After merging ASM_HOLD and ASM_SORTIN
	// ASM: cold cycles = 76261 | warm cycles = 76242-76333			After latency hiding in ASM_SORTIN
	// ASM: cold cycles = 75262 | warm cycles = 75232-75233			After saving WAIT list head to R12
	// ASM: cold cycles = 74851 | warm cycles = 74813				After saving WAIT_LIST.head->right1 to R8 before ASM_CYCLE calling
	// ASM: cold cycles = 72679-72700 | warm cycles = 72557-72616	After ASM_U1 .balign 16
	// ASM: cold cycles = 72346 | warm cycles = 72275-72306			After hoisting LDR from ASM_U2 to ASM_U1
	// ASM: cold cycles = 72086 | warm cycles = 72001-72122			After Loading ASM_E6 and ASM_E3 and then MOVS in ASM_U2
	// ASM: cold cycles = 72071-72097 | warm cycles = 71994-72015	After Latency hiding in ASM_U2_2H_CONTINUE
	// ASM: cold cycles = 71993 | warm cycles = 71910				After Latency hiding in ASM_U6_BODY
	// ASM: cold cycles = 71987 | warm cycles = 71906-72019			After Latency hiding in ASM_U5
	// ASM: cold cycles = 71935-71936 | warm cycles = 71852-71895	After replacing C cycle1 with ASM STR in ASM_E1A
	// ASM: cold cycles = 71768-71770 | warm cycles = 71687			After adding .balign 4 before ASM_E2_LOWER_LOOP
	// ASM: cold cycles = 70140 | warm cycles = 70092-70170			Inlined DELETEW to CYCLE
	// ASM: cold cycles = 69220 | warm cycles = 69154				If C version run before

	start = DWT->CYCCNT;
	uint32_t asm_elevator_status = start_asm_elevator_simulation(max_users);
	if (asm_elevator_status != 0) return asm_elevator_status;
	end = DWT->CYCCNT;
	volatile uint32_t asm_elevator_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_elevator_status = start_asm_elevator_simulation(max_users);
	if (asm_elevator_status != 0) return asm_elevator_status;
	end = DWT->CYCCNT;
	volatile uint32_t asm_elevator_cycles_warm = (end - start) - overhead;

	return 0;
}
