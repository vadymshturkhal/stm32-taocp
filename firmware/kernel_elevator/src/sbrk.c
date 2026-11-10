/* sbrk.c - the heap for malloc (the users' pool), between .bss and the stack
 *
 * newlib's malloc asks _sbrk for memory. The heap starts at "end" (the linker
 * script) and may grow up to the stack's reserved space (_Min_Stack_Size). */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

extern uint8_t end;                /* the first byte after .bss */
extern uint8_t _estack;            /* the top of RAM */
extern uint8_t _Min_Stack_Size;    /* a linker symbol: its address is the size */

void *_sbrk(ptrdiff_t incr)
{
  static uint8_t *heap_end = NULL;
  const uint8_t *limit = &_estack - (uintptr_t)&_Min_Stack_Size;

  if (heap_end == NULL) heap_end = &end;
  if (heap_end + incr > limit)
  {
    errno = ENOMEM;
    return (void *)-1;
  }
  uint8_t *previous = heap_end;
  heap_end += incr;
  return previous;
}
