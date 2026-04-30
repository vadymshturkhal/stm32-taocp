#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// Prototypes
bool run_all_tests(void);
uint8_t c_perform_circular_list_operations(uint32_t max_nodes);

void comparing_circular_lists(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint16_t max_nodes = 128;

	// Run tests
	bool tests_passed = run_all_tests();
	if (!tests_passed) return 0;
	// if (!run_all_tests()) return 0;  // check the result in one line

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 13304 | warm cycles = 13130 | size = ? bytes
	start = DWT->CYCCNT;
	uint8_t gcc_circular_list_status = c_perform_circular_list_operations(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_circular_list_status = c_perform_circular_list_operations(max_nodes);
	if (gcc_circular_list_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_circular_list_cycles_warm = (end - start) - overhead;


	end = DWT->CYCCNT;
}
