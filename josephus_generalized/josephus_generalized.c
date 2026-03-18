#include "main.h"
#include <math.h>


uint32_t josephus_generalized(uint32_t mod, uint32_t participants) {
	uint32_t D = 1;

	while (D <= (mod-1)*participants) {
		D = ceilf( (float)mod/(mod-1) * D );
	}

	return mod*participants + 1 - D;
}
