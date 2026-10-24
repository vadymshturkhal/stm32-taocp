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

## Result (66 users, 1 Knuth unit = 1 ms, `-O2`)

| | |
|---|---|
| Steps / checksum | 544 / 2487227626 (the same as the PC and the earlier board runs) |
| Cycles per step | 210 (≈ 1.2 µs at 170 MHz) |
| Longest step | 691 cycles (≈ 4 µs): the worst blocking for any other step |
| Lateness after the deadline | **max 6 µs, average 301 ns** |
| CPU idle | 99.8 % of the 10.36 s run |

The average lateness is the kernel's wake-up path (TIM2 interrupt, tail-chain
into PendSV, the next deadline read): about 50 cycles. The maximum adds steps
due at the same tick, queued behind each other. Neither depends on
`TICKS_PER_TIME_UNIT`. The run is deterministic: the same binary gives the same
numbers down to the cycle.

### The raw numbers (`elevator_finish()`, Variables view)

| Variable | Value | Meaning |
|----------|------:|---------|
| `steps` | 544 | nodes the kernel ran (`kernel_stats.steps`) |
| `checksum` | 2487227626 | `sum = sum * 31 + TIME` after every step: the same work as the PC |
| `start_cycles` | 8 447 | DWT cycles of the setup: `cpp_elevator_create()` (the users' pool) + `cpp_elevator_start()` |
| `busy_cycles` | 114 648 | DWT cycles inside `kernel.step()`, all steps together |
| `cycles_per_step` | 210 | `busy_cycles / steps` |
| `max_cycles` | 691 | the longest step |
| `late_sum_us` | 164 | µs after the deadline, all steps together (TIM2 ticks, so whole µs per step) |
| `late_avg_ns` | 301 | `late_sum_us * 1000 / steps` |
| `max_late_us` | 6 | the worst lateness |
| `elapsed_us` | 10 361 004 | TIM2 µs from the start to `finished()` |
| `idle_us` | 10 348 837 | µs `main` slept in `__WFI()` |
| `idle_permille` | 998 | `idle_us * 1000 / elapsed_us` |

Awake: `elapsed_us - idle_us` = 12 167 µs, of which the steps are ~0.67 ms
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
