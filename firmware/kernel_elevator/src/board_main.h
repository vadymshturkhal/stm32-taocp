/* board_main.h - the board's program: the C++ elevator on the Knuth kernel
 *
 * All the user code lives in board_main.c, so main.c needs only two lines:
 * this include, and board_main() after the clock (or a CubeMX project's init). */
#ifndef BOARD_MAIN_H
#define BOARD_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Call after the generated init (clock, GPIO). Never returns. */
void board_main(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_MAIN_H */
