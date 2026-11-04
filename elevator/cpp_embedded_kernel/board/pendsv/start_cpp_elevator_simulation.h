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

/* The same elevator in the main loop, on the real clock (1 Knuth unit = 1 ms,
 * TICKS_PER_TIME_UNIT). Do not mix with start_cpp_elevator_simulation() while
 * it runs: both use the same kernel.
 *   cpp_elevator_create():  a fresh elevator, not started yet: the slow part
 *                           (the users' pool), so it can run before "now" is read
 *   cpp_elevator_start():   start it: max_users users from now_ms (creates it
 *                           first if cpp_elevator_create() was not called)
 *   cpp_elevator_stop():    destroy it (the users' pool gives its memory back);
 *                           only once cpp_elevator_running() is 0 */
void cpp_elevator_create(void);
void cpp_elevator_start(uint32_t max_users, uint32_t now_ms);
void cpp_elevator_stop(void);
uint32_t cpp_elevator_pool_refused(void);  /* refused pushes of the users' pool: 0 expected */

/* The trace (with TRACE in the C++ symbols; otherwise they do nothing):
 *   cpp_elevator_trace_poll():    format the waiting records and queue the lines;
 *                                 never waits. Call it from main, not from an interrupt
 *   cpp_elevator_trace_pending(): 1 while records wait
 *   cpp_elevator_trace_dropped(): records lost to a full trace circle: 0 expected */
void cpp_elevator_trace_poll(void);
int cpp_elevator_trace_pending(void);
uint32_t cpp_elevator_trace_dropped(void);

/* Set from PendSV after every elevator step (kernel_port_after_step) */
extern volatile uint32_t elevator_checksum;  /* sum * 31 + TIME: 2487227626 for 66 users */
extern volatile uint32_t elevator_done;      /* 1 once the elevator is finished */  /* TIME after the last step (for the checksum) */

#ifdef __cplusplus
}
#endif

#endif /* START_CPP_ELEVATOR_SIMULATION_H */
