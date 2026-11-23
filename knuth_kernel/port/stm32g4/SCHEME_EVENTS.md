# How the kernel runs on the STM32G4, with events

[SCHEME.md](SCHEME.md) shows how time wakes a kernel level. This file adds the
other way in: an interrupt can wake a node too. The rest of SCHEME.md still holds.

An ISR never touches a WAIT list. It puts the node in its level's circle of buffers
and rings the level; the level's handler, the list's only owner, does the `immed`.
Each level (14: the elevator, 15: PendSV) has its own circle.

**Status:** the code is in and runs on the board with the elevator, but no ISR
calls `kernel_post()` yet. Its first users will be the button and the I2C DMA.

```
                  ┌──────────────────────────────────────────┐
  main (no prio)  │ loop:  Link_Poll                         │
                  │        cpp_elevator_trace_poll           │
                  │        done? → stop, finish              │
                  │        __disable_irq                     │
                  │        kernel_port_sleep ◄── sleeps here │
                  │        __enable_irq                      │
                  └──────────────────────────────────────────┘

  ISR (prio 3: button, I2C, its DMA)       TIM2 (prio 2)
  ┌────────────────────────────────────┐   ┌───────────────────┐
  │ kernel_post(level, node):          │   │ TIM2_IRQHandler:  │
  │   level.kernel.post(node):         │   │   clear CCxIF     │
  │     assign: full? → ++dropped      │   │   ring(level)     │
  │     *slot = &node                  │   └─────────┬─────────┘
  │     release ─────────┐             │             │
  │   ring(level) ───────│───────┐     │             │
  └──────────────────────│───────│─────┘             │
                         ▼       ├───────────────────┘
   events_: 8 slots of WaitNode* │ STIR or PENDSVSET (the level's doorbell)
                         │ red   │
  the level (14, 15)     │       ▼ run_level(level)
  ┌──────────────────────│───────────────────────────────────┐
  │ for (;;) {           │                                   │
  │   t = now()          ▼                                   │
  │   kernel.start_event(t)                                  │
  │     one red slot? → immed(node, t): node goes first      │
  │     none?         → nothing                              │
  │   due = kernel.get_next_time()                           │
  │   kernel.step(t) ── nothing due? ── break                │
  │     due: unlink first, run its step                      │
  │   stats: cycles, lateness                                │
  │ }                                                        │
  │                                                          │
  │ refused_ops, events_dropped → level.stats                │
  │ arm(level): the next alarm                               │
  └──────────────────────────────────────────────────────────┘
```

One event on a timeline:

```
time ───────────────────────────────────────────────────────────────────────────────►

main    ███ WFI (asleep) ···█ woken ██ __enable_irq                       ██ loop, WFI ···
                            ▲          │                                   ▲
button edge ────────────────┘          ▼                                   │
ISR     (pending until __enable_irq)   █ post, ring the level              │
                                        ╲ tail-chain                       │ return
level                                    ██ start_event ██ step ██ arm ────┘
```

The cycle: **post → ring the level → its handler: `start_event`, `step`, per event → `arm`**.

## Rules

1. **Only ISRs of one priority post to a level.** Each level's circle has one
   producer. Two ISRs at the same priority can't preempt each other, so together they
   are one producer. `main` must not post: an ISR could preempt it in the middle of
   `post`.
2. **Only the level's handler calls `start_event`** (in `run_level`). It changes the
   level's WAIT list.
3. **ISRs call `kernel_post(level, node)`, not `level.kernel.post(node)`.** Without
   the ring the event waits for the level's next alarm, or for ever if nothing on
   that level is held.
4. **Post only to a node that outlives the ISR.** An event can wait in the circle
   after its node was cancelled or freed, so the pointer must stay valid. Use nodes
   that live for the whole run.
5. **Keep steps short.** A step can't be interrupted by its own level (a higher
   level can preempt it), so the longest step on a level is how long an event for
   that level can wait, and how fast its circle can fill.

## Edge cases

**Two events at once.** `immed` inserts first, so taking both in one pass would run
the second one first. `start_event` takes one per pass, and `step` runs it before
the next one enters the list, so events run in the order they were posted.

**Events and due nodes.** An event goes ahead of the nodes already due: they wait
until it has run. A steady stream of events would hold them back.

**A node that is holding.** `immed` unlinks it and moves it first. A node has one
`NEXTTIME`, so its old deadline is gone: this is the "timeout or event" wait, and
the step must find out which of the two happened.

**The same node posted twice.** It takes two slots and runs twice.

**An ISR fires during the loop.** It only writes to the circle and rings the level.
The running pass doesn't see it unless it hasn't reached `start_event` yet;
otherwise a later pass or the level's next run takes it. No event is lost.

**An ISR fires after the loop's last pass, before `arm()`.** The ring makes the
level run again at once.

**The circle is full.** `post` returns `false` and counts it in `events_dropped`.
`kernel_post()` rings the level anyway, because the level's run is what makes room.

**Lateness of an event.** `immed` sets `NEXTTIME = t`, so `late` is 0 for it:
`max_late_us` says nothing about events. The time from the edge to the step needs
its own timestamp, taken in the ISR.

## Priorities

| Level | What runs there | Posts? |
|-------|-----------------|--------|
| 2 | TIM2, the alarms | no, it only rings the levels |
| 3 | button (`EXTI15_10`), `I2C1_EV`, `I2C1_ER`, `DMA1_Channel1` | yes: the one producer of the circle it posts to |
| 14 | level 14, `CORDIC_IRQHandler`: the elevator | no, the consumer of its own circle |
| 15 | level 15, PendSV: the trace drain and `tx_watchdog` (phase 3) | no, the consumer of its own circle |

The button's level is `BSP_BUTTON_USER_IT_PRIORITY` in `stm32g4xx_nucleo_conf.h`,
applied by `BSP_PB_Init()`. Its default is 15, the same as PendSV, so the button
couldn't preempt a step. Elevator-Primes moves it to 3 with `NVIC_SetPriority()`
after `BSP_PB_Init()`, in a USER CODE block, so it survives regeneration.

All levels share the one MSP. In the worst case `main`, level 15, level 14, an ISR
at 3 and TIM2 are all on it at once, with a frame for each interrupt (32 bytes, 104
with the FPU context). The static estimate with the two levels and TIM2 nested is
392 of 896 bytes (no ISR posts yet). The [stack guard](SCHEME.md#the-stack-guard-stack_guardc)
turns an overflow into a recorded HardFault.

## New names

| Name | Where | What it does |
|------|-------|--------------|
| `kernel_post(level, node)` | [kernel_port_priorities.hpp](kernel_port_priorities.hpp) | what an ISR calls: `level.kernel.post(node)` + `ring(level)`; `false` if the event was dropped |
| `kernel.post(node)` | [kernel.hpp](../../kernel.hpp) | writes the node's address into the circle: `assign`, store, `release`. Touches no list |
| `kernel.start_event(t)` | [kernel.hpp](../../kernel.hpp) | takes one event: `start_operation`, `immed`, `operation_is_completed` |
| `events_` | [kernel.hpp](../../kernel.hpp) | `CircleOfBuffers<WaitNode*, 8>`: pointers to nodes, the nodes are never copied. 8 is a margin, not a calculation; `events_dropped` checks it |
| `kernel.events_dropped()` | [kernel.hpp](../../kernel.hpp) | events refused because the circle was full; copied to the level's stats at the end of every run |

## Cost

With no event waiting, `start_event` is six instructions on every pass (two loads,
two `dmb`, two branches). That is an estimated 10-15 cycles. It is not in
`cycles_per_step`, because `c0` is read after it; it shows as lateness of a node
waiting behind another one.

Measured with no events posted (4 users, `-O2`, `TRACE`, 921600 baud), before / now:
checksum 4123600382 / the same; `late_avg_ns` 285 / 357; `max_late_us` 6 / 7;
awake 4.62 / 4.60 ms.
