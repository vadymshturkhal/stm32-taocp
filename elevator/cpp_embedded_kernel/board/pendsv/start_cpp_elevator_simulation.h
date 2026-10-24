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
 *   cpp_elevator_create():  a fresh elevator, not started yet: the slow part
 *                           (the users' pool), so it can run before "now" is read
 *   cpp_elevator_start():   start it: max_users users from now_ms (creates it
 *                           first if cpp_elevator_create() was not called)
 *   cpp_elevator_step():    run at most one due node (the kernel's rule: one
 *                           node per pass, so the main loop gets a turn);
 *                           1 if a node ran, 0 if nothing was due
 *   cpp_elevator_step_next(): as fast as possible: run the first node at its
 *                           own time, without waiting for the clock; 1 if a
 *                           node ran, 0 if the WAIT list is empty
 *   cpp_elevator_running(): 1 until the elevator is finished: every user
 *                           gone and the elevator dormant at E1 (not "the
 *                           list is empty": the kernel is shared)
 *   cpp_elevator_stop():    destroy it (the users' pool gives its memory back);
 *                           only once cpp_elevator_running() is 0 */
void cpp_elevator_create(void);
void cpp_elevator_start(uint32_t max_users, uint32_t now_ms);
int cpp_elevator_step(uint32_t now_ms);
int cpp_elevator_step_next(void);
int cpp_elevator_running(void);
void cpp_elevator_stop(void);
uint32_t cpp_elevator_time(void);
uint32_t cpp_elevator_pool_refused(void);  /* refused pushes of the users' pool: 0 expected */

/* Set from PendSV after every elevator step (kernel_port_after_step) */
extern volatile uint32_t elevator_checksum;  /* sum * 31 + TIME: 2487227626 for 66 users */
extern volatile uint32_t elevator_done;      /* 1 once the elevator is finished */  /* TIME after the last step (for the checksum) */

#ifdef __cplusplus
}
#endif

#endif /* START_CPP_ELEVATOR_SIMULATION_H */
