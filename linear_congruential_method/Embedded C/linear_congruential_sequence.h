// linear_congruential_sequence.h
#ifndef LINEAR_CONGRUENTIAL_SEQUENCE_H
#define LINEAR_CONGRUENTIAL_SEQUENCE_H

#include <stdint.h>

typedef struct {
    uint32_t a;
    uint32_t c;
    uint32_t X;
    uint32_t e;
    uint32_t w;  // w = 2^e
    uint32_t m;  // m = w + 1
} LinearCongruentialMethod;

void lcm_init(LinearCongruentialMethod* lcm, uint32_t a, uint32_t c, uint32_t X_0, uint32_t e);
uint32_t lcm_next_number(LinearCongruentialMethod* lcm);
uint32_t lcm_next_number_clever(LinearCongruentialMethod* lcm);

#endif
