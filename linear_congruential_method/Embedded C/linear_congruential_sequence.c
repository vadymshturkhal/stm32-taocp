#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "linear_congruential_sequence.h"

void lcm_init(LinearCongruentialMethod* lcm, uint32_t a, uint32_t c, uint32_t X_0, uint32_t e) {
    lcm->a = a;
    lcm->c = c;
    lcm->X = X_0;
    lcm->e = e;
    lcm->w = (uint32_t)1 << e;
    lcm->m = lcm->w + 1;
}

uint32_t lcm_next_number(LinearCongruentialMethod* lcm) {
    // Computes (aX) mod m, where m = w + 1

    uint32_t X = lcm->X;
    lcm->X = (lcm->a * lcm->X) % lcm->m;
    return X;
}

uint32_t lcm_next_number_clever(LinearCongruentialMethod* lcm) {
    // Computes (aX) mod m, where m = w + 1, using w = -1 (mod m)
    //
    // MIX instructions:
    // LDAN X      # load negative X to rA
    // MUL A       # multiply rA by A, upper bytes are placed in rA, lower in rX
    // STX TEMP
    // SUB TEMP
    // JANN *+3
    // INCA 2
    // ADD =w-l=

    uint32_t X = lcm->X;
    uint32_t product = X * lcm->a;
    uint32_t q = product >> lcm->e;             // MUL: high word q (rA)
    uint32_t r = product & (lcm->w - 1);        // MUL: low word r (rX)
    int32_t A = (int32_t)r - (int32_t)q;        // STX TEMP; SUB TEMP

    if (A < 0) {
        A += 2;             // INCA 2
        A += lcm->w - 1;    // ADD =w-l=
    }

    lcm->X = (uint32_t)A;
    return X;
}
