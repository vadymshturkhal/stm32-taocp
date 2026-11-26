# How the kernel runs on the STM32G4

`main` sleeps, TIM2's two compare channels are the alarms, and two NVIC priority
levels run the kernel. There is one `Kernel`, and it holds a WAIT list per level;
each level also has its own stats:

- **level 14**, run by `CORDIC_IRQHandler` (CORDIC's spare vector): the elevator;
- **level 15**, run by PendSV: the background work, the trace drain and
  `tx_watchdog` (phase 3). It is empty for now.

The levels are a table in [kernel_port_priorities.hpp](kernel_port_priorities.hpp).
Each node knows its level: `kernel.attach<P>(node)` points the node at level P's
WAIT list, and every later `hold`, `holdc`, `immed` and `cancel` on it goes to that
list. Only a level's handler touches that level's list (rule 1), so the levels need
no locks; the NVIC does the preemption.

No ISR can wake a node yet. The events circle that did it was removed: see
[SCHEME_EVENTS.md](SCHEME_EVENTS.md). The button and the I2C DMA will be the first
to need the new way.

**Measured** (the bare build, 66 users): the same 544 steps, checksum 2487227626
and 473 wakes as with the one PendSV kernel, the same maximum lateness, 5 µs, and
the same **average lateness, 0.28 µs** (`late_sum_us` 151), awake 1.32 ms of the
10.36 s. That needs the hot paths (`TIM2_IRQHandler`, `arm`, the race check's
ring) to know their level at compile time: they are templates on the level's
priority, so its row of `level_config` folds into constants (`TIM2_IRQHandler`:
23 instructions). The first version read each level's channel and IRQ from
`levels` at run time (51 instructions in the handler): average lateness 0.77 µs,
awake 1.61 ms. The price of the templates: one copy of the drain loop per level,
+288 bytes of flash.

**One Kernel for both levels** (Elevator-Primes, 66 users, `-flto` on the C and the
C++): `busy_cycles` 121,488, against 121,940 with one `Kernel` per level, and the
same checksum, 5 µs maximum and 0.26 µs average lateness. The 3 cycles a step saved
are the events check (`start_event`) that each pass of the loop no longer makes.
Each operation finds its list through the node's `list_`, one load. Finding it from a
priority stored in the node, through a table, cost 11 to 25 cycles a step (measured
in a scratch build, not in the repo).

```
                  ┌──────────────────────────────────────────┐
  main (no prio)  │ start: stack_guard_init (the MPU guard)  │
                  │        kernel_port_init, create, start   │
                  │        kernel_port_start(): ring every   │
                  │          level once (rule 1 from here)   │
                  │ loop:  Link_Poll                         │
                  │        cpp_elevator_trace_poll           │
                  │        done? → stop, finish              │
                  │        __disable_irq                     │
                  │        kernel_port_sleep ◄─ here         │
                  │        __enable_irq                      │
                  └──────────────────────────────────────────┘

  TIM2 (prio 2), the alarms: CNT == CCR1 → level 14, CNT == CCR2 → level 15
  ┌───────────────────────────────────────────────────────────────────────┐
  │ TIM2_IRQHandler:                                                      │
  │   CC1IF? (is it set?) → clear it, ring_at<14>(): NVIC->STIR = 100     │
  │   CC2IF? (is it set?) → clear it, ring_at<15>(): ICSR.PENDSVSET       │
  └────────────────────────────┬──────────────────────────────────────────┘
                               │ tail-chain, no return to main
                               ▼
  level 14 (prio 14)  CORDIC_IRQHandler → run_level<14>(): the elevator
  level 15 (prio 15)  PendSV_Handler → kernel_PendSV_Handler → run_level<15>()
                      (empty until phase 3); level 14 preempts it

  run_level<P>(), the same loop for every level P (level_at<P>()):
  ┌──────────────────────────────────────────────────────────┐
  │ for (;;) {                                               │
  │   t = now()                                              │
  │   kernel.step<P>(t) ── nothing due? ── break             │
  │     due: unlink first, run its step                      │
  │     (E1..E9, U1..U6: hold, holdc, immed → node's list)   │
  │   stats: cycles, lateness → level.stats                  │
  │ }                                                        │
  │ kernel.refused<P>() → level.stats.refused_ops            │
  │ arm<P>():                                                │
  │   list empty?  → return (no register is touched)         │
  │   its CCRx = first node's NEXTTIME, clear only its CCxIF │
  │   now ≥ NEXTTIME already? → ring_at<P>(): it runs again  │
  └──────────────────────────────────────────────────────────┘

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
TIM2    (pending until __enable_irq)   █ clear CC1IF, ring level 14          │
                                        ╲ tail-chain                         │ return
level 14                                 ██ step ██ step ██ arm ─────────────┘
                                         (two nodes due: drained in one run)
```

The cycle: **arm → sleep → alarm → TIM2 rings the level → drain loop (step,
step, ...) → arm**.

Priorities: TIM2 2 (the highest of these), 3 kept for the ISRs that will wake
nodes, then the kernel levels, 14 and 15. `level_config`'s `static_assert`s keep
every level at 4..15, below the real interrupts, so the kernel never delays one
(`main` masks them only around `__WFI`). Any context may ring a level (a single
register write); no ISR and no other level may change a level's list.

## Edge cases

**A node comes due while another node of the same level runs.** No interrupt:
during the drain loop the level's `CCRx` still holds the old alarm (already
passed); `arm()` sets the new one only after the loop. The loop catches it instead:
it reads `now()` again after every step, so the node runs right after the current
one, in the same run. Its lateness is the rest of the current step (what
`max_late_us` measures). Steps of one level run to completion: the next node never
interrupts the current one.

```
time    ──────────────|B due|──────────────────────►
level   ██ A ████████████████ ██ B ██ arm
                      └─ late ─┘ = the rest of A
```

**A node comes due after the loop's last check, before `arm()` arms the alarm.**
The compare matches only on equality, so arming a time already passed would wait
71.6 min for the wrap. `arm()`'s race check (`now ≥ NEXTTIME` after arming) sees it
and rings the level: it runs again at once.

**TIM2 matches during a drain.** It happens whenever a level runs for another
reason while its `CCRx` holds a deadline still ahead: `main`'s
`kernel_port_start()`, or `arm()`'s race check (later also an ISR waking a node).
If the drain is still running when `CNT` reaches `CCRx`, TIM2 (priority 2) preempts
it for a moment and rings the level again. After this run ends, the level runs once
more, finds nothing due and arms again. One wasted pass, harmless. Once the button
and I2C wake nodes, it is no longer rare.

**Level 14 comes due while level 15 runs.** TIM2 rings level 14, which preempts
level 15 at once, between or inside a step; level 15 goes on where it was after
level 14 returns. The levels share nothing but TIM2, and they split it: each has its
own `CCRx`, clears only its own `CCxIF` (one write, rc_w0), and neither writes
`DIER`. Level 15's step cycles then include level 14's run.

**A level's list becomes empty.** `arm()` returns without touching a register, so
the level's `CCRx` keeps its old time. Its next match, after the 71.6 min wrap,
rings the level once: it finds nothing and arms nothing. One wasted pass. Turning
the alarm off would need a `DIER` write, a read-modify-write that the other level
could preempt and undo; so both enables are set once, in `kernel_port_init()`.

## Every name in the scheme

### `main` ([main.c](../../../elevator/cpp_embedded_kernel/board/pendsv/main.c))

| Name | What it does |
|------|--------------|
| `stack_guard_init()` | ([stack_guard.c](stack_guard.c)) first, before the clock: paints the stack and puts the MPU guard under it. In the bare build (`firmware/kernel_elevator/src/main.c`) and Elevator-Primes; not yet in this `main.c`. See [The stack guard](#the-stack-guard-stack_guardc) |
| `kernel_port_init()` | starts TIM2 as the 1 µs clock (PSC = 169, 32-bit, free-running); clears `CC1IF` and `CC2IF` and sets `DIER` = `CC1IE` \| `CC2IE`, once; sets each level's priority from `level_config` and enables its IRQ (CORDIC; PendSV is always enabled); TIM2 at 2 |
| `cpp_elevator_create()` | builds the elevator on `kernel` and its users' pool (the `malloc` and every node constructed once); before reading "now", so the setup is not counted as lateness |
| `cpp_elevator_start()` | Knuth's TIME 0 = now: attaches ELEV1..3 and USER1 to level 14 (`ELEVATOR_LEVEL`) and `immed`s USER1, the sole entry in level 14's WAIT list (as in Knuth); the elevator's nodes are started later by the users' steps, and a user's node is attached when U1 takes it from the pool |
| `kernel_port_start()` | the kick, after the setup: rings every level once. Until here `main` owns the WAIT lists; from here on only each level's handler touches its own (rule 1). An empty level runs once and arms nothing |
| `Link_Poll()` | the Pi link's work: reads the bytes the UART interrupt stored, collects a line, at `'\n'` checks its CRC and replies; checks the link's timeouts |
| `cpp_elevator_trace_poll()` | `TRACE` builds: turns the records level 14's steps wrote into lines and queues them for the UART's DMA. Here, not in a level, because formatting is slow; the levels can preempt it |
| `__disable_irq()` | PRIMASK = 1: interrupts still happen, but their handlers wait (pending) |
| `kernel_port_sleep()` | ([kernel_port.cpp](kernel_port.cpp)) `__DSB(); __WFI(); __ISB()` and returns the µs slept. Wait For Interrupt: the CPU sleeps until an interrupt is pending; it wakes even while they are masked. The function runs from SRAM (section `.data_ramfunc`, copied with `.data`): with `DBG_SLEEP` set, the G431 can skip the first instruction fetched from flash after the wake (ES0431 §2.2.7, seen here even with its workarounds) |
| `__enable_irq()` | PRIMASK = 0: the pending handlers (TIM2, then level 14, then level 15) run at this line. Masked check + sleep + enable = no lost wake-up |
| `done? → stop, finish` | `main` checks `cpp_elevator_finished()`, the atomic flag the elevator sets when it is finished; then it reads `cpp_elevator_checksum()`, stops the elevator (`cpp_elevator_stop()`: frees the users' pool; none of its nodes is in level 14's list) and calls `elevator_finish()` (collects the numbers) |

### The levels ([kernel_port_priorities.hpp](kernel_port_priorities.hpp))

| Name | What it does |
|------|--------------|
| `level_config` | the `constexpr` table, one row per level: `priority`, `irq` (its handler), `channel` (its alarm): {14, `CORDIC_IRQn`, 0} and {15, `PendSV_IRQn`, 1}. The priorities come from `kernel_priorities` |
| `kernel_priorities` | ([kernel_levels.hpp](kernel_levels.hpp)) the levels' priorities, {14, 15}: the only place they are written. `kernel.hpp` takes them from here (a WAIT list for each; the PC builds, which don't have this file on their include path, use the same two values), and `level_config`'s rows refer to them. A new level: a priority here, a row in `level_config`, its stats global and its handler |
| the `static_assert`s | every priority 4..15 (`first_kernel_priority`: below TIM2 and the posting ISRs); every channel 0 or 1; no priority, IRQ or channel used twice. A wrong table fails the build |
| `level_index(p)` | `consteval`: the table's row for priority `p`; a priority with no level is a compile error |
| `kernel` | `constinit inline Kernel kernel`: the one kernel, a WAIT list per level (`std::array<WaitList, 2>`), built at compile time |
| `kernel.attach<P>(node)` | puts the node on level P: sets its `list_` to level P's WAIT list (only while the node waits in no list). Every `hold`, `holdc`, `immed` and `cancel` then goes to that list, with one load. A node that was never attached fails an assert in debug builds |
| `KernelLevel` | a level's changing part: a reference to its stats (a C global) and one to its row of `level_config` |
| `levels` | `constinit std::array` of the two levels: built at compile time, no constructor runs before `main`. Each entry finds its config by priority |
| `level_14`, `level_15` | the names programs use: `levels[0]`, the elevator; `levels[1]`, the trace drain and `tx_watchdog` (phase 3) |
| `kernel_stats_level_14`, `_15` | each level's `KernelStats`; `main.c`, GDB and `setup.sh` read them |
| `ring(level)` | the doorbell: makes the level's handler pending. CORDIC: `NVIC->STIR = CORDIC_IRQn`; PendSV: `pend_kernel()`, i.e. `SCB->ICSR = PENDSVSET` (a system exception: `STIR` can't pend it). Called by `kernel_port_start()`. Several rings before it runs = one run |
| `level_at<P>()`, `ring_at<P>()` | the compile-time versions, for code that knows its level's priority `P`: the level's row of `level_config` is then a constant, so its channel and IRQ fold into the code. The hot paths use them: `TIM2_IRQHandler`, `run_level<P>`, `arm<P>`. `level_at<P>()` is `levels[level_index(P)]`, so a priority with no level is a compile error |
| `NVIC->STIR`, `SCB->ICSR` | single writes, never a read-modify-write, so any context can ring safely; writing 0 to `ICSR`'s other bits does nothing |

### TIM2, the clock and the alarms

| Name | What it does |
|------|--------------|
| `TIM2->CNT` | the counter: µs since start, 32 bits, wraps after 71.6 min. `now()` reads it |
| `TIM2->CCR1`, `CCR2` | the alarm times of level 14 (channel 0) and level 15 (channel 1). When `CNT == CCRx`, the compare matches |
| `CC1IF`, `CC2IF` | the compare flags in `TIM2->SR`, set by the match; each must be cleared, or the interrupt repeats for ever. They are rc_w0: writing 0 clears, writing 1 changes nothing, so `SR = ~flag` clears one level's flag in one write without touching the other's (or `UIF`) |
| `CC1IE`, `CC2IE` | the compare interrupt enables in `TIM2->DIER`: both set once in `kernel_port_init()` and never written again |
| `TIM2_IRQHandler` | ([kernel_port.cpp](kernel_port.cpp)) the alarms' interrupt: one `check_alarm<P>()` per level, expanded at compile time from `level_config`: if the level's flag is set, clear it and `ring_at<P>()`. In HAL builds it also counts TIM2's wraps (`UIF`) for `HAL_GetTick` |
| `HAL_Tick_From_TIM2()` | HAL builds only (`USE_HAL_DRIVER`): `HAL_GetTick` in ms from TIM2's µs and its wraps, and SysTick off, so there are no 1 ms wake-ups. Called once, before the kernel starts: its read-modify-write of `DIER` (adding `UIE`) is safe then, with interrupts masked |
| tail-chain | the Cortex-M goes from one handler straight into the next pending one (TIM2 → level 14 → level 15) without returning to `main` in between |

### The levels' handlers and the drain loop ([kernel_port.cpp](kernel_port.cpp))

| Name | What it does |
|------|--------------|
| `CORDIC_IRQHandler` | level 14: `run_level<14>()`. CORDIC's vector used as a spare software interrupt; the CORDIC itself stays off. Defined in the port, it replaces the startup file's weak alias, so CubeMX needs no change |
| `PendSV_Handler` | level 15's exception handler (in `stm32g4xx_it.c` in the CubeIDE projects, in `board_main.c` in the bare build); only calls `kernel_PendSV_Handler()` |
| `kernel_PendSV_Handler()` | `run_level<15>()`. `extern "C"`, so the C handler can call this C++ function |
| the handlers' `static_assert`s | the table must name `CORDIC_IRQn` for level 14 and `PendSV_IRQn` for level 15: the vector table fixes which handler runs which level |
| `run_level<P>()` | one run of level P (`level_at<P>()`): the drain loop on level P's list (`kernel.step<P>`), the stats into `level.stats`, then `arm<P>()`. A template on the priority, so the level is known at compile time; each level gets its own copy, inlined into its handler. A higher level can preempt it between or inside steps |
| `now()` | `TIM2->CNT`: the current time in µs, read again on every pass of the loop |
| `kernel.step<P>(t)` | level P's list, found at compile time (a fixed offset in `kernel`): if its first node is due (its time ≤ `t`): unlinks it, runs its step, returns `true`; otherwise `false`, and the loop ends. Knuth's CYCLE on the real clock. "≤" is wrap-safe, `!is_less(t, NEXTTIME)` (the sign of the difference): it holds while deadlines are less than 2^31 µs (35.8 min) apart |
| E1..E9, U1..U6 | the elevator's and the users' steps (TAOCP §2.2.5). Most end with `hold`, `holdc` or `immed`, which change the WAIT list. Not all: E1 is empty (the elevator is dormant until a user's step starts it), E2 can jump straight to E3, and U6 gives the user's node back to the pool |
| `hold(node, now, delay)` | puts the node back in its level's WAIT list (`node.list_`), due `delay` from now (sorted in by time) |
| `holdc(node, now, delay, step)` | the same, with a new step to run next |
| `immed(node, now)` | puts the node first in the list: it runs in this same drain loop |
| stats: cycles, lateness | DWT cycles of the step (`busy_cycles`, `max_cycles`); `now - deadline` in µs (`late_sum_us`, `max_late_us`). A step's cycles include any higher level that ran inside it. The elevator's average lateness is 0.28 µs and the maximum 5 µs, as with the one PendSV kernel (0.77 µs while the alarm path read the level table at run time) |
| `kernel.refused<P>()` | misuse level P refused in release builds (a free node held or cancelled, a due node with no step); 0 in a correct run. `kernel.refused()` is the sum over the levels |
| `arm<P>()` | level P's next alarm (a template like `run_level<P>`, so the channel and IRQ are constants): list empty → return, no register touched; else the level's `CCRx` = the first node's `NEXTTIME` (the earliest deadline: the list is sorted by time; `kernel.get_next_time<P>()`, i.e. `head.right`, not the sentinel `head` itself), clear only its `CCxIF`, then the race check: if `now()` is already at or past the deadline, `ring_at<P>()` (a missed match comes back only after the 71.6 min wrap) |
| `NEXTTIME` | a node's deadline in µs (Knuth's field name) |

### The stack guard ([stack_guard.c](stack_guard.c))

`main`, TIM2, the ISRs and both kernel levels all run on the one MSP, and each interrupt
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
| `stack_guard_depth()` | the deepest the stack has gone since `stack_guard_init()`: scans up from the guard to the first word that is not the paint. `ElevatorRun.stack_used` in the bare build: 220 of 896 bytes with the two levels (228 with the one PendSV kernel) |
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
