#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "simulation.h"

int main(int argc, char** argv) {
	uint32_t users_quantity = 4;
	if (argc > 1) {
		users_quantity = (uint32_t)strtoul(argv[1], NULL, 10);
	}

#ifdef TRACE
	printf("TIME STATE FLOOR D1 D2 D3 step action\n");
#endif

	uint32_t status = start_elevator_simulation(users_quantity);
	if (status != 0) {
		fprintf(stderr, "start_elevator_simulation failed: status %u\n", status);
		return 1;
	}

	return 0;
}
