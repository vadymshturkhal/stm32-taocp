#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "linear_congruential_sequence.h"

uint32_t test_linear_congruential_method(void) {
    LinearCongruentialMethod lcm;
    lcm_init(&lcm, 7, 0, 7, 8);

    uint32_t sequence_size = 44;
    uint32_t sequence[sequence_size];

    for (uint32_t i = 0; i < sequence_size; i++) {
    	sequence[i] = lcm_next_number_clever(&lcm);
    }


    return 0;
}
