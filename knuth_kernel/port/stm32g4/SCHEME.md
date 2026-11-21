# How the kernel runs on the STM32G4

`main` sleeps, the TIM2 compare is the alarm, PendSV runs the kernel.

An ISR can ring the doorbell too: it posts a node with `kernel_post()`, and each
pass of the loop first takes one posted event (`start_event`).
[SCHEME_EVENTS.md](SCHEME_EVENTS.md) has that part, its rules and its edge cases. No
ISR posts yet: the button and the I2C DMA will be the first.

```
                  ┌──────────────────────────────────────────┐
  main (no prio)  │ start: stack_guard_init (the MPU guard)  │
                  │        kernel_port_init, create, start   │
                  │        pend_kernel() ────────────┐       │
                  │ loop:  Link_Poll                 │       │
                  │        cpp_elevator_trace_poll   │       │
                  │        done? → stop, finish      │       │
                  │        __disable_irq             │       │
                  │        kernel_port_sleep ◄─ here │       │
                  │        __enable_irq              │       │
                  └──────────────────────────────────│───────┘
                                                     │ PENDSVSET (the doorbell)
  TIM2 (prio 2)                                      │
  ┌──────────────────────────┐                       │
  │ TIM2_IRQHandler:         │  CNT == CCR1          │
  │   clear CC1IF            │◄──── (the alarm) ─────│───────────┐
  │   pend_kernel() ─────────│───────────────────────┤           │
  └──────────────────────────┘                       │           │
                       tail-chain, no return to main │           │
                                                     ▼           │
  PendSV (prio 15)   PendSV_Handler → kernel_PendSV_Handler()    │
  ┌──────────────────────────────────────────────────────────┐   │
  │ for (;;) {                                               │   │
  │   t = now()                         ◄──────────────┐     │   │
  │   kernel.start_event(t): one event → immed         │     │   │
  │   kernel.step(t) ── nothing due? ── break ──┐      │     │   │
  │     │ due: unlink first, run its step       │      │     │   │
  │     │      (E1..E9, U1..U6: hold, holdc,    │      │     │   │
  │     │       immed → the WAIT list changes)  │      │     │   │
  │     ▼                                       │      │     │   │
  │   stats: cycles, lateness                   │      │     │   │
  │ }  ─────────────────────────────────────────│──────┘     │   │
  │                                             ▼            │   │
  │   refused_ops, events_dropped → kernel_stats_level_14    │   │
  │   pend_next():                                           │   │
  │     list empty?  → CC1IE off, return (no alarm)          │   │
  │     CCR1 = first node's NEXTTIME ────────────────────────│───┘
  │     clear CC1IF, CC1IE on                                │
  │     now ≥ NEXTTIME already? → pend_kernel() ──┐          │
  └───────────────────────────────────────────────│──────────┘
                                                  │ PendSV again at once
   return → main goes on after __enable_irq       │ (a deadline missed by pend_next)
                                                  ▼
                                         back to kernel_PendSV_Handler

  The application (the elevator), inside its own steps:
  ┌──────────────────────────────────────────────────────────┐
  │ begin_step():  checksum = checksum * 31 + TIME           │
  │ check_done():  finished()? → done = true (atomic)        │
  │   at the steps that can end the run: U6, E6B, E9         │
  │   main: cpp_elevator_finished() ──► stop, finish         │
  │ (only the elevator's steps count, whatever else runs)    │
  └──────────────────────────────────────────────────────────┘
```

One deadline on a timeline:

```
time ──────────────────────────────────────────────────────────────────────────────────────►

main    ███ WFI (asleep) ···█ woken ██ __enable_irq                         ██ loop, WFI ···
                            ▲          │                                     ▲
           CNT == CCR1 ─────┘          ▼                                     │
TIM2    (pending until __enable_irq)   █ clear flag, pend_kernel             │
                                        ╲ tail-chain                         │ return
PendSV                                   ██ step ██ step ██ pend_next ───────┘
                                         (two nodes due: drained in one run)
```

The cycle: **pend_next → sleep → alarm → TIM2 pends PendSV → drain loop (event?,
step, event?, step, ...) → pend_next**. An ISR's `kernel_post()` enters the same
cycle at "pends PendSV".

Priorities: TIM2 2 (higher), the ISRs that post 3, PendSV 15 (the lowest), so the
kernel never delays an interrupt (`main` masks them only around `__WFI`), and only
PendSV touches the WAIT list: no locks. ISRs reach it only through the event
circle.

## Edge cases

**A node comes due while another node runs.** No interrupt: during the drain
loop `CCR1` still holds the old alarm (already passed); `pend_next()` sets the new one
only after the loop. The loop catches it instead: it reads `now()` again after
every step, so the node runs right after the current one, in the same PendSV.
Its lateness is the rest of the current step (what `max_late_us` measures).
Steps run to completion: the next node never interrupts the current one.

```
time    ──────────────|B due|──────────────────────►
PendSV   ██ A ███████████████ ██ B ██ pend_next
                      └─ late ─┘ = the rest of A
```

**A node comes due after the loop's last check, before `pend_next()` arms the
alarm.** The compare matches only on equality, so arming a time already passed
would wait 71.6 min for the wrap. `pend_next()`'s race check (`now ≥ NEXTTIME` after
arming) sees it and calls `pend_kernel()`: PendSV runs again at once.

**TIM2 matches during a drain.** It happens whenever PendSV starts for another
reason while `CCR1` holds a deadline still ahead: an ISR's `kernel_post()`, `main`'s
first `pend_kernel()`, or `pend_next()`'s race check. If the drain is still running when
`CNT` reaches `CCR1`, TIM2 (priority 2) preempts PendSV for a moment and sets PendSV
pending again. After this run ends, PendSV runs once more, finds nothing due and
aims again. One wasted pass, harmless. Once the button and I2C post, it is no
longer rare.

## Every name in the scheme

### `main` ([main.c](../../../elevator/cpp_embedded_kernel/board/pendsv/main.c))

| Name | What it does |
|------|--------------|
| `stack_guard_init()` | ([stack_guard.c](stack_guard.c)) first, before the clock: paints the stack and puts the MPU guard under it. In the bare build (`firmware/kernel_elevator/src/main.c`) and Elevator-Primes; not yet in this `main.c`. See [The stack guard](#the-stack-guard-stack_guardc) |
| `kernel_port_init()` | starts TIM2 as the 1 µs clock (PSC = 169, 32-bit, free-running) and sets the priorities: TIM2 2, PendSV 15 |
| `cpp_elevator_create()` | builds the elevator and its users' pool (the `malloc` and every node constructed once); before reading "now", so the setup is not counted as lateness |
| `cpp_elevator_start()` | Knuth's TIME 0 = now: `immed`s USER1, the sole entry in the WAIT list (as in Knuth); the elevator's nodes are started later by the users' steps |
| `Link_Poll()` | the Pi link's work: reads the bytes the UART interrupt stored, collects a line, at `'\n'` checks its CRC and replies; checks the link's timeouts |
| `cpp_elevator_trace_poll()` | `TRACE` builds: turns the records PendSV's steps wrote into lines and queues them for the UART's DMA. Here, not in PendSV, because formatting is slow; PendSV can preempt it |
| `__disable_irq()` | PRIMASK = 1: interrupts still happen, but their handlers wait (pending) |
| `kernel_port_sleep()` | ([kernel_port.cpp](kernel_port.cpp)) `__DSB(); __WFI(); __ISB()` and returns the µs slept. Wait For Interrupt: the CPU sleeps until an interrupt is pending; it wakes even while they are masked. The function runs from SRAM (section `.data_ramfunc`, copied with `.data`): with `DBG_SLEEP` set, the G431 can skip the first instruction fetched from flash after the wake (ES0431 §2.2.7, seen here even with its workarounds) |
| `__enable_irq()` | PRIMASK = 0: the pending handlers (TIM2, then PendSV) run at this line. Masked check + sleep + enable = no lost wake-up |
| `done? → stop, finish` | `main` checks `cpp_elevator_finished()`, the atomic flag the elevator sets when it is finished; then it reads `cpp_elevator_checksum()`, stops the elevator (`cpp_elevator_stop()`: frees the users' pool; none of its nodes is in the kernel) and calls `elevator_finish()` (collects the numbers) |

### The doorbell ([kernel_port.hpp](kernel_port.hpp))

| Name | What it does |
|------|--------------|
| `pend_kernel()` | `SCB->ICSR = SCB_ICSR_PENDSVSET_Msk`: makes PendSV pending, as if hardware had raised it. The only way into the kernel; called by `main` at the start, `TIM2_IRQHandler` on the alarm, and `pend_next()` for a deadline already past. Several calls before PendSV runs = one run |
| `SCB->ICSR` | the core's Interrupt Control and State Register; writing 0 to its other bits does nothing, so a plain `=` is safe |
| `PENDSVSET` | the ICSR bit that sets PendSV pending |

### TIM2, the clock and the alarm

| Name | What it does |
|------|--------------|
| `TIM2->CNT` | the counter: µs since start, 32 bits, wraps after 71.6 min. `now()` reads it |
| `TIM2->CCR1` | Capture/Compare Register 1: the alarm time. When `CNT == CCR1`, the compare matches |
| `CC1IF` | Capture/Compare 1 Interrupt Flag (in `TIM2->SR`): set by the match; must be cleared, or the interrupt repeats for ever |
| `CC1IE` | Capture/Compare 1 Interrupt Enable (in `TIM2->DIER`): the alarm on or off |
| `TIM2_IRQHandler` | ([kernel_port.cpp](kernel_port.cpp)) the alarm's interrupt: if `CC1IF` is set and `CC1IE` is on (an alarm armed), clears `CC1IF` and calls `pend_kernel()`. In HAL builds it also counts TIM2's wraps (`UIF`) for `HAL_GetTick` |
| `HAL_Tick_From_TIM2()` | HAL builds only (`USE_HAL_DRIVER`): `HAL_GetTick` in ms from TIM2's µs and its wraps, and SysTick off, so there are no 1 ms wake-ups. Called once, before the kernel starts |
| tail-chain | the Cortex-M goes from one handler straight into the next pending one (TIM2 → PendSV) without returning to `main` in between |

### PendSV, the kernel ([kernel_port.cpp](kernel_port.cpp))

| Name | What it does |
|------|--------------|
| `PendSV_Handler` | the PendSV exception's handler (in `stm32g4xx_it.c` in the CubeIDE projects, in `board_main.c` in the bare build); only calls `kernel_PendSV_Handler()` |
| `kernel_PendSV_Handler()` | the kernel's body: the drain loop, then `pend_next()`. `extern "C"`, so the C handler can call this C++ function |
| `now()` | `TIM2->CNT`: the current time in µs, read again on every pass of the loop |
| `kernel.start_event(t)` | takes one posted event, if there is one, and `immed`s its node: it runs ahead of the nodes already due. See [SCHEME_EVENTS.md](SCHEME_EVENTS.md) |
| `kernel.step(t)` | if the WAIT list's first node is due (its time ≤ `t`): unlinks it, runs its step, returns `true`; otherwise `false`, and the loop ends. Knuth's CYCLE on the real clock. "≤" is wrap-safe, `!is_less(t, NEXTTIME)` (the sign of the difference): it holds while deadlines are less than 2^31 µs (35.8 min) apart |
| E1..E9, U1..U6 | the elevator's and the users' steps (TAOCP §2.2.5). Most end with `hold`, `holdc` or `immed`, which change the WAIT list. Not all: E1 is empty (the elevator is dormant until a user's step starts it), E2 can jump straight to E3, and U6 gives the user's node back to the pool |
| `hold(node, now, delay)` | puts the node back in the WAIT list, due `delay` from now (sorted in by time) |
| `holdc(node, now, delay, step)` | the same, with a new step to run next |
| `immed(node, now)` | puts the node first in the list: it runs in this same drain loop |
| stats: cycles, lateness | DWT cycles of the step (`busy_cycles`, `max_cycles`); `now - deadline` in µs (`late_sum_us`, `max_late_us`) |
| `kernel.refused()` | misuse the kernel refused in release builds (a free node held or cancelled, a due node with no step); 0 in a correct run |
| `pend_next()` | sets the next alarm: `CC1IE` off if the list is empty; else `CCR1` = the first node's `NEXTTIME` (the earliest deadline: the list is sorted by time; `kernel.get_next_time()`, i.e. `head.right`, not the sentinel `head` itself), clear `CC1IF`, `CC1IE` on, then the race check: if `now()` is already at or past the deadline, `pend_kernel()` (a missed match comes back only after the 71.6 min wrap) |
| `NEXTTIME` | a node's deadline in µs (Knuth's field name) |

### The stack guard ([stack_guard.c](stack_guard.c))

`main`, TIM2, the ISRs and PendSV all run on the one MSP, and each interrupt
that nests stacks its frame on top of the one it preempted. The Cortex-M4 has
no stack-limit register, so the guard is an MPU region at the stack's bottom.

```
_estack  0x20008000  ┬ the stack: 896 of the 1 KB reserve, grows down; MSP starts here
                     │ painted 0xDEADBEEF, measured by stack_guard_depth()
         0x20007C80  ├ the guard: 128 bytes, no access (MPU region 0)
         0x20007C00  ┴ the heap stops here (sbrk)
```

| Name | What it does |
|------|--------------|
| `stack_guard_init()` | returns with the guard off if the stack's bottom is not aligned to the guard's size. Paints the stack from above the guard up to 16 words below the current SP (its own frame) with `STACK_PAINT` (`0xDEADBEEF`). Then it sets MPU region 0 over the guard: no access (`AP` = 0), execute never (`XN`), size `_Stack_Guard_Size` from the linker script; `PRIVDEFENA` keeps the default map for the rest. Sets `is_stack_guard_on` |
| `_Stack_Guard_Size` | 128, set in the `.ld`, with `ASSERT`s: a power of 2 ≥ 32, the stack's bottom aligned to it, smaller than the stack. 128, not 32: a frame with the FPU context is 104 bytes, so a frame that starts above the guard ends inside it, never below it in the heap |
| `stack_guard_depth()` | the deepest the stack has gone since `stack_guard_init()`: scans up from the guard to the first word that is not the paint. `ElevatorRun.stack_used` in the bare build (228 of 896 bytes) |
| `HardFault_Handler` | an alias of `stack_guard_hard_fault` (naked; in the CubeIDE projects `stm32g4xx_it.c` branches to it). A push into the guard is a MemManage fault; MemManage is not enabled, so it escalates to HardFault. The entry moves MSP back to `_estack` (the old SP points into the guard) and passes the old SP and `EXC_RETURN` to `stack_guard_record()` |
| `stack_guard_record()` | fills `stack_guard_fault`: CFSR, HFSR, MMFAR/BFAR (only when valid), LR and PC from the frame (when it was stacked), the old SP, `EXC_RETURN`, `count`. Then `BKPT` if a debugger is attached, else it loops for ever |
| `stack_guard_fault` | the record, read by the debugger or `./setup.sh results`; `./setup.sh test` passes only with `is_stack_guard_on` = 1 and `count` = 0 |

### The application, the elevator ([start_cpp_elevator_simulation.cpp](../../../elevator/cpp_embedded_kernel/board/pendsv/start_cpp_elevator_simulation.cpp))

| Name | What it does |
|------|--------------|
| `begin_step()` | ([elevator_simulation.hpp](../../../elevator/cpp_embedded_kernel/elevator_simulation.hpp)) the start of every elevator step: sets `TIME` from the node and adds it to the checksum. A step that jumps into another one calls its body (`E3_impl`, `E7_impl`, `E8_impl`), so each kernel step counts once, and other programs' steps don't count |
| `checksum = checksum * 31 + TIME` | `ElevatorSimulation::checksum`, a fingerprint of the run: every step's `TIME`, in order. `measurements/board/main.c` computes the same sum after each step; the PC build (`pc/main.cpp`) has none, so the PC is checked by its trace instead |
| `TIME` | Knuth's simulated time, set from the node at the start of each step |
| `finished()` | every user gone and the elevator dormant at E1: none of its nodes is in the WAIT list |
| `check_done()` | at the steps that can end a run (U6, E6B going dormant, E9): `finished()`? → `done = true`, a `std::atomic<bool>` stored with release; `main` reads it with acquire through `cpp_elevator_finished()`, so the checksum written before it is visible too |
| `elevator_finish()` | in `main.c`: copies every number into local variables for the debugger (the breakpoint goes there) |
