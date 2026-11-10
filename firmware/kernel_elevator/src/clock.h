/* clock.h - the system clock without the HAL */
#ifndef CLOCK_H
#define CLOCK_H

/* HSI 16 MHz -> PLL -> 170 MHz (range 1 boost, 4 flash wait states).
 * Call first in main. */
void clock_170mhz(void);

#endif /* CLOCK_H */
