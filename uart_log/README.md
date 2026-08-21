# uart_log

DMA-driven, non-blocking UART logging. Replaces the blocking, byte-by-byte
`HAL_UART_Transmit()` used by `__io_putchar`/`printf` with a queued,
interrupt-drained transfer, freeing the CPU from busy-waiting during trace
output. Measured ~65x fewer CPU cycles per call site versus blocking
`printf` on this project's elevator simulation.

## Setup (one-time, per project)

1. **Call `uart_log_init(&hcom_uart[COM1])` once**, after `BSP_COM_Init()`
   succeeds and before any `async_printf()` call. Currently done in `main()`.

2. **Wire both interrupt handlers** in `stm32g4xx_it.c`:
   ```c
   void DMA1_Channel1_IRQHandler(void) { uart_log_dma_irq_handler(); }
   void LPUART1_IRQHandler(void)       { uart_log_uart_irq_handler(); }
   ```
   Both are required. Without `LPUART1_IRQHandler` specifically, only the
   very first message ever transmits — everything after it sits queued
   forever, since nothing calls `HAL_UART_TxCpltCallback` to drain it.
   (Why two handlers: in `DMA_NORMAL` mode, the DMA-complete interrupt only
   arms the UART's own completion interrupt; it doesn't fire the drain
   callback directly. See the comment in `uart_log_init()`.)

3. **Add `-ICore/Src/uart_log` to any file's include path** that isn't
   already inside `Core/Src/uart_log/` and needs the bare `#include
   "uart_log.h"` form. Files directly in `Core/Src/` (like `main.c`) can
   use `#include "uart_log/uart_log.h"` instead, which resolves without
   the extra include path. Add the path via Project Properties → C/C++
   Build → Settings → MCU GCC Compiler → Include paths — hand-editing the
   generated `subdir.mk` doesn't persist, it gets wiped on the next
   CubeIDE regenerate.

## Using it

```c
async_printf(buf, len);          // fire-and-forget, queues + dispatches
uart_log_print_remaining();      // optional: block until fully drained
```

- `async_printf(const uint8_t* buf, size_t len)` takes a **raw
  pre-formatted buffer**, not a format string — no `printf`-style
  varargs. If you have dynamic content, build the line with
  `snprintf`/`vsnprintf` into a local buffer first (see `trace.c` for the
  pattern), then pass that buffer and its actual length.
- Silently drops the whole message if the ring buffer doesn't have room —
  no partial/corrupted lines, but no error either. Not expected under
  this project's current trace volume (4096-byte buffer).
- `uart_log_print_remaining()` blocks until everything queued has
  genuinely transmitted. Only needed where you require certainty (before
  a reset/power-down, between measurement phases for clean cycle counts)
  — not needed for ordinary logging.

## Gotchas

- **Don't mix in plain blocking `printf()`** on the same UART alongside
  `async_printf()` calls. They're two unsynchronized transmission paths —
  a blocking call can transmit ahead of still-queued async output, and
  running it while DMA is mid-transfer is a real hardware conflict, not
  just a display ordering issue.
- **Only one process attached to `/dev/ttyACM0` at a time** when viewing
  output on the host (check with `lsof`/`fuser` if unsure). Linux doesn't
  enforce exclusive access to serial devices by default — two readers
  (e.g. STM32CubeIDE's Terminal view left open alongside an external
  terminal) each get a partial, colliding slice of the same byte stream.
  This produces a "clean ordered subsequence, zero bit corruption"
  garbling pattern that looks exactly like a firmware bug and cost a very
  long debugging session before the real cause was found.

## Internals (for anyone touching `uart_log.c`)

Byte ring buffer, not the `UartLogNode`/`WAIT_LIST` design sketched in
`uart_log_list.c/h` (that stayed a design exercise and was never wired
in). Three fixes worth knowing about if you're modifying the dispatch
logic:

- **Signed-arithmetic trap**: `uint16_t` operands promote to signed `int`
  in C's usual arithmetic conversions, so a bare `(tail - head) % SIZE`
  goes negative and gives a wrong, huge result once reassigned to
  `uint16_t` when `tail` has wrapped past `head`. Always compute distance
  as `(tail - head + SIZE) % SIZE`.
- **Split-transfer**: DMA needs one contiguous range, but the logical
  pending distance can wrap past the buffer's physical end.
  `uart_log_dispatch_next()` caps each transfer at the buffer end; the
  wrapped remainder drains on the next completion.
- **Atomic tail publish**: `async_printf()` writes into a *local* copy of
  `tail` during its copy loop and publishes the final value to the shared
  `uart_log_tail` in one atomic store at the end — a per-byte update to
  the shared variable would let the DMA completion ISR read a
  partially-advanced `tail` mid-copy and dispatch a transfer including
  stale bytes from a previous message.
