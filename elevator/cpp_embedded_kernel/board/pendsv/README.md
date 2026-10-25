# The elevator in real time on the STM32G4: PendSV, SysTick on

Knuth's CYCLE (TAOCP Vol. 1, §2.2.5) jumps from event to event in simulated
time. Here the same elevator runs on a NUCLEO-G431RB in **real time**: the
[Knuth kernel](../../../../knuth_kernel) runs in **PendSV**, and the
**TIM2 compare** wakes it at each deadline. Between deadlines the CPU sleeps in
`__WFI()`; `main` only runs the [Pi ↔ MCU link](../../../../pi_mcu_protocol).

The elevator does the same work as on the PC, to the last bit: the same 544
steps, the same checksum of TIME.

The kernel itself is tickless (it wakes only at deadlines), but HAL's 1 ms
SysTick still runs for `HAL_GetTick()` (the link's timeouts), so the CPU also
wakes 1000 times a second. A tickless variant (SysTick off) comes later in a
folder beside this one.

## Result (66 users, 1 Knuth unit = 1 ms)

Every build does the same work: 544 steps, checksum 2487227626 (the same as the
PC and the earlier board runs), no refused operations, CPU idle 99.8 % of the
10.36 s run.

| Build (whole project) | `-O2` | `-O3` | `-O3 -flto` |
|---|---:|---:|---:|
| Cycles per step | 233 (1.37 µs) | 226 (1.33 µs) | **219** (1.29 µs) |
| Longest step | 718 (4.2 µs) | 698 (4.1 µs) | **652** (3.8 µs) |
| Lateness after the deadline: max / average | 6 µs / 303 ns | 6 µs / 301 ns | **5 µs / 275 ns** |
| Flash: the elevator's steps | **4 112 B** | 4 854 B | 5 022 B |
| Flash: the kernel's handlers (PendSV, TIM2) | **264 B** | 264 B | 352 B |
| Flash: the elevator and the kernel | **4 376 B** | 5 118 B | 5 374 B |

- **Cycles** are DWT cycles inside `kernel.step()` (one node, its step and the
  list work), at 170 MHz. The longest step is the worst blocking for any other
  step.
- **The elevator's steps** are the functions of `Elevator`, `Users`,
  `ElevatorSimulation` and `ElevatorList` (`arm-none-eabi-nm -S`); the kernel is
  header-only, so its list code is inlined into them. With LTO the helpers
  (`decision`, `E6B`, `U2`, ...) are inlined into the steps too.
- **The kernel's handlers** are `kernel_pendsv` (the drain loop with
  `Kernel::step`, `aim()` and the stats), `PendSV_Handler` and
  `TIM2_IRQHandler`; with LTO `kernel_pendsv` is inlined into
  `PendSV_Handler`. `kernel_port_init` (84 B, run once) is left out: with LTO
  it is inlined into `main`.
- `-O3 -flto` buys 6 % (14 cycles per step) for 23 % more flash than `-O2`.

**The kernel's misuse guards** (a free node held or cancelled, a due node with no
step; see [the kernel README](../../../../knuth_kernel)) cost 24 cycles per step:
the same `-O2` build with the kernel headers from before the guards runs at 209
cycles per step (longest 688).

The average lateness is the kernel's wake-up path (TIM2 interrupt, tail-chain
into PendSV, the next deadline read): about 50 cycles. The maximum adds steps
due at the same tick, queued behind each other. Neither depends on
`TICKS_PER_TIME_UNIT`. The run is deterministic: the same binary gives the same
numbers down to the cycle.

### The raw numbers (`-O2`, `elevator_finish()`, Variables view)

| Variable | Value | Meaning |
|----------|------:|---------|
| `steps` | 544 | nodes the kernel ran (`kernel_stats.steps`) |
| `checksum` | 2487227626 | `sum = sum * 31 + TIME` after every step: the same work as the PC |
| `start_cycles` | 8 446 | DWT cycles of the setup: `cpp_elevator_create()` (the users' pool) + `cpp_elevator_start()` |
| `busy_cycles` | 127 072 | DWT cycles inside `kernel.step()`, all steps together |
| `cycles_per_step` | 233 | `busy_cycles / steps` |
| `max_cycles` | 718 | the longest step |
| `late_sum_us` | 165 | µs after the deadline, all steps together (TIM2 ticks, so whole µs per step) |
| `late_avg_ns` | 303 | `late_sum_us * 1000 / steps` |
| `max_late_us` | 6 | the worst lateness |
| `elapsed_us` | 10 361 004 | TIM2 µs from the start to `finished()` |
| `idle_us` | 10 348 777 | µs `main` slept in `__WFI()` |
| `idle_permille` | 998 | `idle_us * 1000 / elapsed_us` |
| `refused_ops` | 0 | misuse the kernel refused (a free node held or cancelled, a due node with no step): must be 0 |
| `pool_refused` | 0 | pushes the users' pool refused (a free, held or foreign node): must be 0 |

Awake: `elapsed_us - idle_us` = 12 227 µs, of which the steps are ~0.75 ms
(`busy_cycles` / 170); the rest is mostly SysTick's 1 ms wake-ups (~1 µs each).

## How it runs

```
TIM2 CNT == CCR1 ─► TIM2_IRQHandler: clear the flag, pend PendSV
                         │
       (no other interrupt active) ─► PendSV_Handler ─► kernel_pendsv():
                                        run every due node (the drain loop)
                                        kernel_port_after_step() after each one
                                        aim(): CCR1 = the next deadline
main:  Link_Poll ─► elevator_done? ─► __WFI() ─► (woken) ─► ...
```

- **Only PendSV touches the kernel** after the first `pend_kernel()`: `main`
  and the ISRs never call `hold` or `cancel`, so the list needs no locks.
- **PendSV has the lowest priority** (15), TIM2 a higher one (2): the kernel
  never delays an interrupt.
- **`aim()` re-checks the deadline** after arming the compare: if it already
  passed, it pends PendSV itself (otherwise the next match is 71.6 min away).
- **The end is the elevator's own state** (`finished()`: every user gone, the
  elevator dormant at E1), not an empty list: the kernel is shared.

## Files

| File | Goes to (CubeIDE project) | What it is |
|------|---------------------------|------------|
| `main.c` | `Core/Src/main.c` | starts the link and the elevator, waits for `elevator_done`, sleeps in `__WFI()`; collects every number in `elevator_finish()` |
| `start_cpp_elevator_simulation.{h,cpp}` | `Core/Src/elevator_cpp/` | the elevator callable from C: `cpp_elevator_create()` / `cpp_elevator_start()` / `cpp_elevator_stop()`, and the kernel hook `kernel_port_after_step()` (checksum, `elevator_done`) |

From elsewhere in the repository:

| Source | Goes to |
|--------|---------|
| [`../..`](../..) (`*.cpp`, `*.hpp`, not `pc/`) | `Core/Src/elevator_cpp/` |
| [`knuth_kernel/*.hpp`](../../../../knuth_kernel) | `Core/Src/knuth_kernel/` |
| [`knuth_kernel/port/stm32g4/`](../../../../knuth_kernel/port/stm32g4) | `Core/Src/knuth_kernel/port/stm32g4/` |
| [`pi_mcu_protocol`](../../../../pi_mcu_protocol) MCU side (`link`, `uart_rx`, `uart_tx`, `crc16`, `app`) | `Core/Src/` |

## Setting up the project

1. A CubeMX project for the NUCLEO-G431RB (170 MHz from the PLL, APB1 not
   divided, so TIM2 counts 1 µs with `PSC = 169`), **converted to C++**.
2. **Include paths**, for both the C and the C++ compiler:
   `../Core/Src/knuth_kernel` and `../Core/Src/knuth_kernel/port/stm32g4`.
3. **C++ flags:** GNU++20 or later, `-fno-exceptions -fno-rtti
   -fno-use-cxa-atexit -fno-threadsafe-statics`; `NDEBUG` for measurements.
4. **The PendSV hook** in `Core/Src/stm32g4xx_it.c` (CubeMX generates
   `PendSV_Handler`; the call goes in its USER CODE block, so it survives
   regeneration):
   ```c
   /* USER CODE BEGIN Includes */
   #include "kernel_port.hpp"
   ...
   void PendSV_Handler(void)
   {
     /* USER CODE BEGIN PendSV_IRQn 0 */
     kernel_pendsv();
   ```
   `TIM2_IRQHandler` is defined in `kernel_port.cpp` (TIM2 is not configured in
   the `.ioc`, so CubeMX does not generate one).
5. `TICKS_PER_TIME_UNIT` in `elevator_constants.hpp`: 1000 (1 Knuth unit =
   1 ms, a 10.4 s run). Any value gives the same steps and checksum.

## Running it

1. **Clean**, **Build**, start a **Debug** session.
2. One breakpoint in `elevator_finish()`, on the `__NOP()` line; **Resume**.
3. After ~10 s it stops there. The Variables view shows `steps`, `checksum`,
   `start_cycles`, `cycles_per_step`, `max_cycles`, `late_sum_us`,
   `late_avg_ns`, `max_late_us`, `elapsed_us`, `idle_us`, `idle_permille`.

`DBGMCU_CR_DBG_SLEEP` keeps the debugger connected while the CPU sleeps.
Measuring by single-stepping does not work here: the debugger masks interrupts
while stepping, so PendSV seems to run later than it does.

---

*Code, design and measurements are mine; Claude helped draft the READMEs and the paper, and reviewed code.*
