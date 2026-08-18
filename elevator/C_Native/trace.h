#ifndef TRACE_H
#define TRACE_H

#include "elevator_settings.h"

// Table 1 style trace line: "TIME STATE FLOOR D1 D2 D3 step action"
// Matches Python's row format exactly (see elevator_wait_list/elevator.py)
void trace(SharedState* shared_state, const char* step, const char* fmt, ...);

#endif
