/* cstyle_loop.h - the C-style C++ elevator in the main loop, callable from C
 *
 * The same shape as c_elevator_loop.h:
 *   cstyle_elevator_start(): malloc, init, users_start(); 0, or an error status
 *   cstyle_elevator_step():  one pass of cycle(): 1 if a node ran, 0 at the end
 *   cstyle_elevator_stop():  free the memory
 *   cstyle_elevator_time():  TIME after the last step (for the checksum)
 */
#ifndef CSTYLE_LOOP_H
#define CSTYLE_LOOP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t cstyle_elevator_start(uint32_t max_users);
int cstyle_elevator_step(void);
void cstyle_elevator_stop(void);
uint32_t cstyle_elevator_time(void);

#ifdef __cplusplus
}
#endif

#endif /* CSTYLE_LOOP_H */
