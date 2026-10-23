/* c_elevator_loop.h - the C elevator in the main loop, one node per pass
 *
 * The same run as start_elevator_simulation(), split so the main loop gets a
 * turn between nodes (the C counterpart of cpp_elevator_start/step_next/stop):
 *   c_elevator_start(): malloc, init, users_start(); 0, or an error status
 *   c_elevator_step():  one pass of cycle(): run the first node of the WAIT
 *                       list at its own time; 1 if a node ran, 0 if the list
 *                       is empty (the end)
 *   c_elevator_stop():  free the memory
 */
#ifndef C_ELEVATOR_LOOP_H
#define C_ELEVATOR_LOOP_H

#include <stdint.h>

uint32_t c_elevator_start(uint32_t max_users);
int c_elevator_step(void);
void c_elevator_stop(void);
uint32_t c_elevator_time(void);    /* TIME after the last step (for the checksum) */

#endif /* C_ELEVATOR_LOOP_H */
