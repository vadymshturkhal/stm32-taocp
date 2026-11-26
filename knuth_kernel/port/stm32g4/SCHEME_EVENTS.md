# Events: the way an interrupt wakes a node

[SCHEME.md](SCHEME.md) shows how time wakes a kernel level. This file is about the
other way in: an interrupt (or another level) waking a node.

**Status:** none for now. The first design, an events circle per level, was removed
on 2026-11-26 without ever having a user. The next one is not decided yet. The
button and the I2C DMA will be its first users, then the trace drain on level 15,
woken by the elevator on level 14.

## What was here, and why it went

Each level's `Kernel` had an events circle (`CircleOfBuffers<WaitNode*, 8>`). An ISR
called `kernel_post(level, node)`: the node's address into the level's circle, then
`ring(level)`. Each pass of `run_level` took one event first (`start_event`) and
`immed`ed its node, so only the level's handler changed its list (rule 1).

It went for two reasons:

1. **One producer per circle.** `post` is three steps, `assign` → store the
   pointer → `release`, and they are not atomic together. Two posters at different
   priorities get the same slot: the second `release` is refused, and one event is
   lost without a sound. The planned posters are more than one per level (level 14
   and the button would both wake level 15's nodes), so the rule would have needed
   a circle per poster.
2. **One `Kernel` for every level.** The kernel now holds a WAIT list per level, and
   each node knows its list (`kernel.attach<P>(node)`). The circle belonged to the
   old one-`Kernel`-per-level layout.

Its cost was a check on every pass of the drain loop, also with no event posted:
removing it saved about 3 cycles a step (`busy_cycles` 121,940 → 121,488,
Elevator-Primes, 66 users).

## What still holds

- **Ringing is safe from anywhere.** `ring(level)` is one register write
  (`NVIC->STIR` or `SCB->ICSR`), so any ISR or level can make a level run. What no
  other context may do is change that level's WAIT list (rule 1).
- **`immed` on a node that waits** unlinks it and puts it first. A node has one
  `NEXTTIME`, so its old deadline is gone: the step must find out whether it woke
  for its timeout or for the event.
- **An event's lateness** isn't in `max_late_us`: `immed` sets `NEXTTIME` to now,
  so `late` is 0. The time from the edge to the step needs a timestamp taken in the
  ISR.
- **The button's priority.** `BSP_BUTTON_USER_IT_PRIORITY` in
  `stm32g4xx_nucleo_conf.h` (applied by `BSP_PB_Init()`) defaults to 15, the same as
  PendSV, so the button couldn't preempt a step. Elevator-Primes moves it to 3 with
  `NVIC_SetPriority()` after `BSP_PB_Init()`, in a USER CODE block, so it survives
  regeneration. 3 is still free for the ISRs that will wake nodes: `level_config`'s
  `static_assert`s keep every kernel level at 4..15.

## The direction being considered

A bit per wakeable node in a pending word per level. To wake a node: `fetch_or` of
its bit (LDREX/STREX, safe for any number of posters at any priority), then
`ring(level)`. The level's handler takes the whole word with `exchange(0)` and
`immed`s each node, highest bit first (CLZ). A second wake before the handler runs
merges with the first; the order is the bits' order, not the order of arrival; a
level has up to 32 such nodes per word. Not decided, not in the code.
