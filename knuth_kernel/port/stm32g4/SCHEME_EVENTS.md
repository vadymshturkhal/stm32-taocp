# How the kernel runs on the STM32G4, with events

[SCHEME.md](SCHEME.md) shows how time wakes the kernel. This file is the current
picture: an interrupt can now wake a node too. The rest of SCHEME.md still holds.

An ISR never touches the WAIT list. It puts the node in a circle of buffers and
rings the doorbell; PendSV, the list's only owner, does the `immed`.

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
  │ kernel_post(node):                 │   │ TIM2_IRQHandler:  │
  │   kernel.post(node):               │   │   clear CC1IF     │
  │     assign: full? → ++dropped      │   │   pend_kernel()   │
  │     *slot = &node                  │   └─────────┬─────────┘
  │     release ─────────┐             │             │
  │   pend_kernel() ─────│───────┐     │             │
  └──────────────────────│───────│─────┘             │
                         ▼       ├───────────────────┘
   events_: 8 slots of WaitNode* │ PENDSVSET (the doorbell)
                         │ red   │
  PendSV (prio 15)       │       ▼ kernel_PendSV_Handler()
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
  │ refused_ops, events_dropped copied to kernel_stats       │
  │ pend_next(): the next alarm                              │
  └──────────────────────────────────────────────────────────┘
```

One event on a timeline:

```
time ───────────────────────────────────────────────────────────────────────────────►

main    ███ WFI (asleep) ···█ woken ██ __enable_irq                       ██ loop, WFI ···
                            ▲          │                                   ▲
button edge ────────────────┘          ▼                                   │
ISR     (pending until __enable_irq)   █ post, pend_kernel                 │
                                        ╲ tail-chain                       │ return
PendSV                                   ██ start_event ██ step ██ pend_next
```

The cycle: **post → doorbell → PendSV: `start_event`, `step`, per event → `pend_next`**.

## Rules

1. **Only ISRs of one priority level post.** The circle has one producer. Two ISRs
   at the same level can't preempt each other, so together they are one producer.
   `main` must not post: an ISR could preempt it in the middle of `post`.
2. **Only PendSV calls `start_event`.** It changes the WAIT list.
3. **ISRs call `kernel_post()`, not `kernel.post()`.** Without the doorbell the event
   waits for the next alarm, or for ever if nothing is held.
4. **Post only to a node that outlives the ISR.** An event can wait in the circle
   after its node was cancelled or freed, so the pointer must stay valid. Use nodes
   that live for the whole run.
5. **Keep steps short.** A step can't be interrupted by the kernel, so the longest
   step is how long an event can wait, and how fast the circle can fill.

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

**An ISR fires during the loop.** It only writes to the circle and sets PendSV
pending. The running pass doesn't see it unless it hasn't reached `start_event`
yet; otherwise a later pass or the next PendSV takes it. No event is lost.

**An ISR fires after the loop's last pass, before `pend_next()`.** The doorbell makes
PendSV run again at once.

**The circle is full.** `post` returns `false` and counts it in `events_dropped`.
`kernel_post()` rings the doorbell anyway, because the kernel is what makes room.

**Lateness of an event.** `immed` sets `NEXTTIME = t`, so `late` is 0 for it:
`max_late_us` says nothing about events. The time from the edge to the step needs
its own timestamp, taken in the ISR.

## Priorities

| Level | What runs there | Posts? |
|-------|-----------------|--------|
| 2 | TIM2, the alarm | no, it only pends PendSV |
| 3 | button (`EXTI15_10`), `I2C1_EV`, `I2C1_ER`, `DMA1_Channel1` | yes, the one producer |
| 15 | PendSV, the kernel | no, the one consumer |

The button's level is `BSP_BUTTON_USER_IT_PRIORITY` in `stm32g4xx_nucleo_conf.h`,
applied by `BSP_PB_Init()`. Its default is 15. Check it after CubeMX regenerates.

All levels share the one MSP. In the worst case `main`, PendSV, an ISR at 3 and
TIM2 are all on it at once, with a frame for each interrupt (32 bytes, 104 with
the FPU context). The [stack guard](SCHEME.md#the-stack-guard-stack_guardc)
turns an overflow into a recorded HardFault.

## New names

| Name | Where | What it does |
|------|-------|--------------|
| `kernel_post(node)` | [kernel_port.hpp](kernel_port.hpp) | what an ISR calls: `kernel.post(node)` + `pend_kernel()`; `false` if the event was dropped |
| `kernel.post(node)` | [kernel.hpp](../../kernel.hpp) | writes the node's address into the circle: `assign`, store, `release`. Touches no list |
| `kernel.start_event(t)` | [kernel.hpp](../../kernel.hpp) | takes one event: `start_operation`, `immed`, `operation_is_completed` |
| `events_` | [kernel.hpp](../../kernel.hpp) | `CircleOfBuffers<WaitNode*, 8>`: pointers to nodes, the nodes are never copied. 8 is a margin, not a calculation; `events_dropped` checks it |
| `kernel.events_dropped()` | [kernel.hpp](../../kernel.hpp) | events refused because the circle was full; copied to `kernel_stats` at the end of every PendSV |

## Cost

With no event waiting, `start_event` is six instructions on every pass (two loads,
two `dmb`, two branches). That is an estimated 10-15 cycles. It is not in
`cycles_per_step`, because `c0` is read after it; it shows as lateness of a node
waiting behind another one.

Measured with no events posted (4 users, `-O2`, `TRACE`, 921600 baud), before / now:
checksum 4123600382 / the same; `late_avg_ns` 285 / 357; `max_late_us` 6 / 7;
awake 4.62 / 4.60 ms.
