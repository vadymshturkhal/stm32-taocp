# The elevator on the STM32G4 board

The [C++ elevator](..) on the [Knuth kernel](../../../knuth_kernel), as
applications for a NUCLEO-G431RB CubeIDE project. Each folder is one variant;
they share the elevator sources, the kernel and its
[STM32G4 port](../../../knuth_kernel/port/stm32g4).

| Folder | Variant | Result (66 users, 1 ms per Knuth unit, `-O2`) |
|--------|---------|------------------------------------------------|
| [`pendsv/`](pendsv) | the kernel in PendSV, woken by the TIM2 compare; HAL's 1 ms SysTick still on | 544 steps, checksum 2487227626, lateness max 6 µs / average 301 ns, CPU awake 12.2 ms of 10.36 s |
| `pendsv_tickless/` (later) | the same with SysTick off: `HAL_GetTick()` from TIM2 | measured once: awake 2.65 ms, 475 wake-ups instead of ~10 900, the same lateness |

Each variant's README says which file goes where in the project, how to set it
up and how to read the results.
