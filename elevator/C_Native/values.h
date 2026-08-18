#ifndef VALUES_H
#define VALUES_H

#include <stdint.h>

typedef struct {
    uint32_t IN;
    uint32_t OUT;
    uint32_t GIVEUPTIME;
    uint32_t INTERTIME;
} Values;

void values_seed(uint32_t seed);
Values values(void);

#endif
