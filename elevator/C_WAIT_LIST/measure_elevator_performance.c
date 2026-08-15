#include <stdint.h>
#include <stdbool.h>

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

	// 4 users
	// GCC -O3 9453
	// GCC -O3 -flto 8062
	// ASM: 9037

	// 66 users
	// GCC -O3 90120 - 90294
	// GCC -O3 -flto 76456
	// ASM: 23569

	// 127 users
	// GCC -O3 166947
	// GCC -O3 -flto 142515
	// ASM: 44950

	// 66 users
	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 90209-90294 | warm cycles = 90280-90340
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

	// 66 users
	// ASM: cold cycles = 23979-23985 | warm cycles = 23912-23913
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
