# C vs C++ on the board: the measurement harness

The STM32G4 side of the C vs C++ measurement ([RESULTS.md](../RESULTS.md#embedded-c-vs-c-stm32g4-cortex-m4--dwt-cyccnt-66-users),
[PAPER.md](../PAPER.md)): the files that exist only on the MCU. The three
elevators themselves are the ports in this repository.

| File | Goes to (CubeIDE project) | What it is |
|------|---------------------------|------------|
| `main.c` | `Core/Src/main.c` | runs the three elevators one after another in the main loop, beside the Pi link, and collects every DWT count in `elevator_finish()` |
| `c_elevator_loop.{c,h}` | `Core/Src/elevator/` | the C run split into start / one step / stop (one pass of `cycle()` per call) |
| `start_cpp_elevator_simulation.{h,cpp}` | `Core/Src/elevator_cpp/` | the same for the kernel C++, callable from C (`cpp_elevator_start`, `cpp_elevator_step_next`, ...) |
| `measure_c_cpp_performance.c` | `Core/Src/elevator_cpp/` | optional: the older batch measurement (whole run per call, cold and warm); its call in `main.c` is commented out |

`main.c` is the board project's own (CubeMX-generated, with the measurement in
its `USER CODE` blocks). It also starts the [Pi ↔ MCU link](../../../pi_mcu_protocol)
(`UART_TX_Init`, `UART_RX_Init`, `Link_Init`, `Link_Poll`): the elevators run one
node per main-loop pass beside it, as real firmware would. To measure without
the link, remove those four calls.

## The project layout

On a NUCLEO-G431RB CubeIDE project (STM32G431RB, 170 MHz):

```
Core/Src/main.c               <- board/main.c
Core/Src/elevator/            <- elevator/c_embedded/*         + board/c_elevator_loop.{c,h}
Core/Src/elevator_cstyle/     <- elevator/cpp_cstyle/*         (not pc/)
Core/Src/elevator_cpp/        <- elevator/cpp_embedded_kernel/* (not pc/) + board/start_cpp_elevator_simulation.{h,cpp}
Core/Src/knuth_kernel/        <- knuth_kernel/{wait_list,kernel,storage_pool}.hpp
```

## Setting up the build

1. **Convert the project to C++:** right-click the project, **Convert to C++**
   (a CubeMX project is C-only, and `.cpp` files would not build).
2. **MCU/MPU G++ Compiler** (Properties, C/C++ Build, Settings; Configuration:
   *All configurations*):
   - Language standard: **GNU++20** or later;
   - Include paths: add `../Core/Src/knuth_kernel`;
   - Other flags: `-fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics`.
3. **The same flags for the three elevator folders.** Set them per folder
   (right-click `elevator`, `elevator_cstyle`, `elevator_cpp`, Properties, C/C++
   Build, Settings), **identically**:
   - Optimization: **`-O3`**;
   - Define symbols: **`NDEBUG`** (the C++ asserts compile to nothing);
   - for the LTO run: **`-flto`** in the compiler flags of **all three** folders.
4. **Check the flags really used**, after a build: they are in the generated
   makefiles, one per folder:
   ```
   grep -oE ' -O[0-3sg]| -flto| -DNDEBUG' Debug/Core/Src/elevator/subdir.mk | sort -u
   grep -oE ' -O[0-3sg]| -flto| -DNDEBUG' Debug/Core/Src/elevator_cstyle/subdir.mk | sort -u
   grep -oE ' -O[0-3sg]| -flto| -DNDEBUG' Debug/Core/Src/elevator_cpp/subdir.mk | sort -u
   ```
   This is the step that matters most: runs 4-11 of the history in RESULTS.md
   compared C built with `-flto` against C++ built without it.

## Running it

1. **Clean** and **Build**, then start a **Debug** session (it flashes the board).
2. **Remove stale breakpoints** (Breakpoints view, *Remove All*): a breakpoint left
   in the main loop stops on every pass.
3. Set **one breakpoint** in `elevator_finish()` in `main.c`, on the `__NOP()` line.
4. **Resume (F8).** The three runs take a few milliseconds; the debugger stops in
   `elevator_finish()`.
5. Read the **Variables** view:

| Variable | Meaning |
|---|---|
| `c_*`, `cs_*`, `cpp_*` | C, C-style C++, kernel C++: `steps`, `start_cycles` (setup), `busy_cycles` (all steps), `total_cycles`, `cycles_per_step`, `checksum` |
| `same_run` | **must be 1**: all three checksums (`sum = sum * 31 + TIME` after every step) and step counts agree, so all three did the same work |

Expected with 66 users: **544** steps each, and about **198 / 198 / 197** cycles
per step at `-O3`, **142 / 140 / 171** at `-O3 -flto`. Differences of about ±1k
cycles per run come from code placement and are not significant.

While it runs, the totals can also be watched in **Live Expressions**
(`elevator_c`, `elevator_cstyle`, `elevator_cpp`, `elevator_phase`): they are
globals, so they survive `-O3`/LTO and need no breakpoint.
