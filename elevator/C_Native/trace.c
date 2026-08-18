#include <stdio.h>
#include <stdarg.h>

#include "trace.h"
#include "elevator.h"

void trace(SharedState* shared_state, const char* step, const char* fmt, ...) {
#ifdef TRACE
	Elevator* elevator = shared_state->elevator;
	const char* state = elevator->STATE > 0 ? "U" : elevator->STATE < 0 ? "D" : "N";

	printf("%04u   %-5s %-4u %-2u %-2u %-2u  %s ",
	       shared_state->TIME, state, elevator->FLOOR,
	       elevator->D1, elevator->D2, elevator->D3, step);

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("\n");
#else
	(void)shared_state;
	(void)step;
	(void)fmt;
#endif
}
