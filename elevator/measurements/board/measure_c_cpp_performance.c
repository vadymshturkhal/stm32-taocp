/* measure_c_cpp_performance.c - DWT cycles of the C elevator vs the C++ elevator (Knuth kernel)
 *
 * The same method as measure_elevator_performance.c (ASM vs C): DWT->CYCCNT
 * around one complete run, minus the cost of reading CYCCNT twice; a cold run
 * (the first one) and a warm run (the same work again) for each version.
 * Both versions run the same 66 users from seed 1, so they do the same work.
 *
 * Read the results in a debug session: put a breakpoint on the final return and
 * look at the volatile *_cycles_* variables.
 *
 * For a fair comparison build both with the same flags: the same -O level
 * (e.g. -O3), no TRACE, and NDEBUG (the C++ asserts compile to nothing).
 * The order matters a little (flash prefetch/ART warm-up, see RESULTS.md):
 * swap the two blocks once to check that the numbers do not depend on it.
 */
#include <stdint.h>
#ifdef TRACE
#include <stdio.h>
#endif

#include "main.h"
#include "start_cpp_elevator_simulation.h"

uint32_t start_elevator_simulation(uint32_t max_users);     /* the C version */

uint32_t measure_c_cpp_performance(void) {
	/* The cycle counter: on without a debugger too */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint32_t max_users = 66;

	/* C (GCC) */
#ifdef TRACE
	printf("TIME STATE FLOOR D1 D2 D3 step action\r\n");
#endif
	start = DWT->CYCCNT;
	uint32_t status = start_elevator_simulation(max_users);
	end = DWT->CYCCNT;
	if (status != 0) return status;
	volatile uint32_t c_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	status = start_elevator_simulation(max_users);
	end = DWT->CYCCNT;
	if (status != 0) return status;
	volatile uint32_t c_cycles_warm = (end - start) - overhead;

	/* C++ on the Knuth kernel */
#ifdef TRACE
	printf("TIME STATE FLOOR D1 D2 D3 step action\r\n");
#endif
	start = DWT->CYCCNT;
	status = start_cpp_elevator_simulation(max_users);
	end = DWT->CYCCNT;
	if (status != 0) return status;
	volatile uint32_t cpp_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	status = start_cpp_elevator_simulation(max_users);
	end = DWT->CYCCNT;
	if (status != 0) return status;
	volatile uint32_t cpp_cycles_warm = (end - start) - overhead;

	/* 66 users
	 * C:   cold cycles = ? | warm cycles = ?
	 * C++: cold cycles = ? | warm cycles = ?
	 */
	(void)c_cycles_cold;
	(void)c_cycles_warm;
	(void)cpp_cycles_cold;
	(void)cpp_cycles_warm;

	return 0;
}
