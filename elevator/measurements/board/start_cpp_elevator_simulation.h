/* start_cpp_elevator_simulation.h - the C++ elevator (Knuth kernel), callable from C
 *
 * The counterpart of the C start_elevator_simulation(): runs max_users users
 * from a fresh state to the end (the WAIT list empty) and returns 0, the same
 * shape as the C function, so the two can be measured side by side.
 */
#ifndef START_CPP_ELEVATOR_SIMULATION_H
#define START_CPP_ELEVATOR_SIMULATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t start_cpp_elevator_simulation(uint32_t max_users);

/* The same elevator in the main loop, on the real clock (1 Knuth unit = 1 ms,
 * TICKS_PER_TIME_UNIT). Do not mix with start_cpp_elevator_simulation() while
 * it runs: both use the same kernel.
 *   cpp_elevator_start():   a fresh elevator, max_users users from now_ms
 *   cpp_elevator_step():    run at most one due node (the kernel's rule: one
 *                           node per pass, so the main loop gets a turn);
 *                           1 if a node ran, 0 if nothing was due
 *   cpp_elevator_step_next(): as fast as possible: run the first node at its
 *                           own time, without waiting for the clock; 1 if a
 *                           node ran, 0 if the WAIT list is empty (the end)
 *   cpp_elevator_running(): 1 while the WAIT list has nodes
 *   cpp_elevator_stop():    destroy it (the users' pool gives its memory back) */
void cpp_elevator_start(uint32_t max_users, uint32_t now_ms);
int cpp_elevator_step(uint32_t now_ms);
int cpp_elevator_step_next(void);
int cpp_elevator_running(void);
void cpp_elevator_stop(void);
uint32_t cpp_elevator_time(void);  /* TIME after the last step (for the checksum) */

#ifdef __cplusplus
}
#endif

#endif /* START_CPP_ELEVATOR_SIMULATION_H */
