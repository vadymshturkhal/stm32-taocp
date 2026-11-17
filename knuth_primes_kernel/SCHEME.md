# How primes reach the OLED on the kernel

Knuth's Algorithm P as a second program beside the elevator: one prime per frame,
each printed on an SSD1306 over I2C with DMA. The button pauses it. Nothing here may
change the elevator's numbers.

[SCHEME_EVENTS.md](../knuth_kernel/port/stm32g4/SCHEME_EVENTS.md) shows how an ISR
wakes a node. This file is what runs on top of that.

**Status:** this is mostly the design. `KnuthPrimes` is here. `OledNode`, `NumberView`
(one fixed number), the font, the port and their PC tests are outside this repo for
now, in `Pi-Board-Elevator/oled/`. `PrimesNode` still holds itself and will lose that
(see [Names](#names)). `PrimesView` and `ButtonNode` are not written. Nothing here has
run on the board yet.

```
  main (no prio)      loop, WFI                     no part in this

  ISR (prio 3)
  ┌───────────────────┐  ┌──────────────────────┐
  │ button EXTI       │  │ I2C done / I2C error │
  │  within 50 ms of  │  │  transfer: busy to   │
  │  the last edge?   │  │  done or failed,     │
  │  → ignore         │  │  else no post        │
  └─────────┬─────────┘  └──────────┬───────────┘
            └────── kernel_post ────┘
                         │ node pointer only
                         ▼
  PendSV (prio 15), one step at a time, no locks
  ┌────────────────────────────────────────────────────────────┐
  │                                                            │
  │  ButtonNode ── pause() / resume() ──┐                      │
  │                                     ▼                      │
  │  OledNode<PrimesView> ── calls ──▶ PrimesView              │
  │    frame_start                       snapshot()            │
  │    page_sent                         render(page, out)     │
  │    end_frame ──────────────────────▶ frame_done(now, ok)   │
  │       │                                │         │         │
  │       │                     hold, 0    │         │ reads   │
  │       │                                ▼         ▼         │
  │       │                          PrimesNode ─▶ KnuthPrimes │
  │       │                          next_prime()  the table   │
  └───────┼────────────────────────────────────────────────────┘
          │ send_page
          ▼
     I2C1 + DMA ─────▶ SSD1306
```

One frame, with L = 0.5 s:

```
time ─────────────────────────────────────────────────────────────────────────►

        │◀──────────────────────── L = 0.5 s ───────────────────────▶│
        │ about 95 ms on the bus │                                   │
OLED    █ window █ p0 █ p1 ... █ p7                                  █ window ...
                               ▲ end_frame: frame_done, hold the rest of L
primes                          █ one next_prime
CPU                                █ asleep ··························█
```

The cycle: **frame timer → snapshot, window → 8 pages, one event each → `end_frame` →
one prime → sleep until the next frame**.

## The frame

| Step | Woken by | What it does |
|------|----------|--------------|
| `frame_start` | the timer, every L | `view.snapshot()`, sends the 6-byte window command, holds 50 ms |
| `page_sent` | an I2C event, or the 50 ms hold | done? `render` the next page into the 128-byte buffer, send it, hold 50 ms. After page 7, or on error or timeout, `end_frame` |
| `end_frame` | called from the steps above | counts the result, `view.frame_done(now, ok)`, then `holdc` to `frame_start` for the rest of L |

Nine transfers per frame: the window, then 8 pages of 128 bytes. Horizontal addressing
is set once at init, which may block because the kernel has not started.

`end_frame` is the only way out of a frame, so no path can miss `frame_done`.

## The view

```cpp
void PrimesView::snapshot() noexcept {
    prime_now = table.get_prime(std::min(shown, table.found() - 1));
}

void PrimesView::frame_done(std::uint32_t now, bool ok) noexcept {
    if (paused) return;
    if (ok && shown < table.found()) ++shown;
    if (table.is_done()) return;
    if (!ok || shown == table.found()) kernel.hold(primes_node, now, 0);
}
```

- **`shown` moves only after a good frame,** so a failed frame prints that prime again.
- **A prime is made when the display has caught up,** so a healthy run has no backlog:
  a prime appears one frame after it is made. Without this check the 2 and 3 already in
  the table leave it two frames behind for the whole run.
- **A failed frame makes one anyway,** so a dead display cannot stop the table.

These rules were checked on a model of `frame_done`, not on the real code:

| Case | Every prime printed, in order | Frames from made to shown |
|------|-------------------------------|---------------------------|
| Healthy | yes | 1 |
| 3 frames fail | yes | 4 at worst |
| Display dead for 100 frames | yes | 101 at worst, then it catches up |
| Paused for 50 frames | yes | 1 |
| Display dead for good | no, it never returns; the table still fills | |

## Rules

1. **Only the OLED's frame period is a timer.** `PrimesNode` does not hold itself, so
   there is no delay to tune.
2. **ISRs only post a node, and set the fields of their own transfer.** They never
   touch the table, the view or the list.
3. **All posters stay at priority 3.** The event circle has one producer. See
   [Priorities](#priorities).
4. **Debounce in the ISR, before posting.** Bounces in the circle could drop an I2C
   event. The same node posted twice also runs twice, and pause would toggle twice.
5. **Put a prime step in with `hold`, never `start`.** `hold` with delay 0 goes behind
   the nodes already due. `start` goes ahead of them.
6. **No pixel drawing in a step.** The font is 6x8, stored as columns, so a character
   is a 6-byte copy aligned to a page. A `constexpr` conversion builds it from the
   library's row font, with only the characters used, in flash.
7. **A step renders one page.** That needs a 128-byte buffer, not 1,024 bytes.
8. **`OledNode` knows nothing about primes,** and `KnuthPrimes` knows nothing about the
   kernel. HAL stays behind `send_page`, so a PC test can fake the bus.
9. **Only the button's program asks to pause, and the view decides what that means.**
   No program calls `cancel` on another one.

## Edge cases

**Timeout or event.** The node holds for 50 ms during each transfer. An event moves it
to the front, so its old deadline is gone. A node has one `NEXTTIME`, so `page_sent`
must find out why it ran. One atomic, `transfer`, says: `idle`, `busy`, `done` or
`failed`. It is the only state the ISR writes, and both sides change it with
compare-and-swap, so a completion and a timeout can't both win.

**A late completion.** The transfer timed out, then finished. `transfer` is no longer
`busy`, so the ISR changes nothing and posts nothing. HAL refuses a new transfer while
the old one is out, so a late completion can't be taken for the next transfer's.

**An extra run.** If the ISR fires between `start_event` and `step` in the pass that
runs the timeout, the step sees the completion and moves on, and the event runs the
node again. It then finds `busy` and the deadline not reached, so it holds for the
time left.

**A bus error.** `HAL_I2C_ErrorCallback` marks the transfer `failed` and posts. The
frame ends with `ok` false and the next frame retries. The window command at each
frame start resyncs the display, so a frame cut short leaves no offset.

**A stuck bus.** Nothing completes, so the 50 ms hold ends the frame. HAL stays busy,
so every later `send_page` is refused and each frame ends at once. The screen stays as
it was and primes carries on. Getting the bus back needs a peripheral reset, which is
not written yet.

**`send_page` refuses.** HAL returning `HAL_BUSY` or `HAL_ERROR` ends the frame the same
way.

**Pause.** Frames keep running and show the same prime. `frame_done` returns before it
changes `shown` or makes a prime.

**The table is full.** The last prime stays on the screen, and the frames keep running.
Stopping them is a choice, not a need.

**A slow frame.** `end_frame` holds for the time left in L. If L has passed, it holds
for 0.

## Priorities

| Level | What runs there | Posts? |
|-------|-----------------|--------|
| 2 | TIM2, the alarm | no |
| 3 | button (`EXTI15_10`), `I2C1_EV`, `I2C1_ER`, `DMA1_Channel1` | yes, the one producer |
| 15 | PendSV: every step in this file | no |

Checked in the board project: all four interrupts are at level 3.

Every step runs at one level, so the table needs no lock. If the view and primes move
to different levels at stage 2, `primes_found` needs `std::atomic`. `next_prime` already
stores the prime first and the count after it.

## Numbers

Calculated, not measured:

| What | Value | From |
|------|-------|------|
| Table | 498 prime steps, about 250 s at L = 0.5 s | |
| Worst prime step | 76 divisions, at 2503, about 5.4 µs | a 12-cycle `udiv` |
| One frame on the bus | about 95 ms | `Timing = 0x40B285C2`, about 100 kHz if I2C1 runs at 170 MHz |
| Smallest L | above about 145 ms | the frame plus one 50 ms timeout |
| Events per frame | 9 | the 8-slot circle stays near empty |
| RAM | 2,000 B table, 128 B page buffer | `uint16_t` makes the table 1,000 B |

`HAL_I2C_Mem_Write_DMA` does not poll on this HAL, so it does not block a step. Its cost
in cycles is unmeasured, and it is the likeliest cause if `max_late_us` rises.

## On the board

- The elevator's checksum does not change.
- `events_dropped` stays 0.
- `max_late_us` is compared with the run without primes. A 4-user run is about 1 s, or
  2 frames, so use a longer one.

## Names

| Name | Where | What it does |
|------|-------|--------------|
| `KnuthPrimes` | [knuth_primes.hpp](knuth_primes.hpp) | the table and `next_prime`. No kernel |
| `PrimesNode` | [primes_node.hpp](primes_node.hpp) | one `next_prime` per step. To do: drop the delay, the kernel and the self-hold |
| `PrimesView` | `primes_view.hpp`, to write | the `shown` index, `snapshot`, `render`, `frame_done`, pause |
| `NumberView` | `Pi-Board-Elevator/oled/number_view.hpp` | one fixed number on the screen. It proves the OLED path before primes use it |
| `OledNode<View>` | `Pi-Board-Elevator/oled/oled_node.hpp` | the frame steps. No HAL, no primes |
| `DIGITS` | `Pi-Board-Elevator/oled/oled_font.hpp` | the library's row digits as columns, built at compile time |
| `ButtonNode` | `button_node.hpp`, to write | calls `pause` or `resume` on the view |
| port | `Pi-Board-Elevator/oled/port/stm32g4/oled_port.cpp` | HAL calls, the completion callbacks, the blocking init, the static objects |

## Starting on the board

- **Order:** `oled_start()` before the elevator's start. PendSV has not run yet, so main
  may still touch the WAIT list. After that, an interrupt could run PendSV in the middle
  of main's list operation.
- **The elevator's checksum counts only its own steps.** The port's after-step hook is
  gone: the elevator adds each step's `TIME` in `begin_step()` and sets its `done` flag
  itself, so the OLED's steps can't change the checksum.
- **`kernel_stats`** counts every program's steps, so `steps`, `busy_cycles` and
  `max_cycles` include the OLED's.

## Tests on the PC

`Pi-Board-Elevator/oled/tests/test_oled.cpp` covers the frame, a bus error, a refused
send, a timeout, a late completion, an extra run and the clock wrap, with a fake
`send_page`.

Still to write, with `PrimesView`:

| Fake does | Expected |
|-----------|----------|
| Completes all 9 transfers | one prime per frame, every L |
| Reports an error on page 3 | frame ends, counted, retried next frame, no prime lost |
| Never completes | 50 ms timeout, then the same; the table still reaches 3571 |
| Pause, then resume | same prime on screen, then it carries on |

Sections 1-3 of [test_primes.cpp](tests/test_primes.cpp) stay. Sections 4-7 change with
`PrimesNode`: the late-start and wrap checks move to the OLED's timer.
