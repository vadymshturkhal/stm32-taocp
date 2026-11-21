# The elevator on the Knuth kernel, bare: no HAL, no CubeMX

The C++ elevator on the Knuth kernel for a NUCLEO-G431RB, built from this
repository's sources (`knuth_kernel`, `elevator/cpp_embedded_kernel`) with
**CMSIS only**: no HAL, no CubeMX, no CubeIDE. One script fetches CMSIS,
builds, flashes and checks the run.

```
./setup.sh           # fetch + build -> build/elevator.elf
./setup.sh test      # build, flash, wait for the run, read the numbers: PASS = 544 steps,
                     # checksum 2487227626, 473 wakes, idle within the run, stack guard on, 0 faults
```

Expected, as in the CubeIDE project: 544 steps, checksum 2487227626, no
refused operations; the cycle counts depend on the compiler version.

## How to build

```
./setup.sh
```

The first run fetches CMSIS into `third_party/cmsis` and checks its SHA-256
sums; then CMake configures and builds `build/elevator.elf` and
`build/elevator.map` from the kernel and the elevator in this checkout
(`../..`). Later runs rebuild only what changed.

**The compiler** on Linux (Debian, Ubuntu), from the distribution's packages:

```
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

The C++ headers (`<optional>`, `<atomic>`, `<concepts>`) come from
`libstdc++-arm-none-eabi-newlib`, the C library from `libnewlib-arm-none-eabi`.
The compiler needs GCC >= 10 for C++20 (Ubuntu 22.04: 10.3). It has no GDB:
see "On the board" below. With the compiler on PATH, `./setup.sh` needs no
`TOOLCHAIN_DIR`.

**Another compiler**, for example CubeIDE's GCC 14.3:

```
./setup.sh clean                              # CMake keeps the compiler a build folder was configured with
TOOLCHAIN_DIR=<folder with arm-none-eabi-gcc> ./setup.sh build
```

**Plain CMake**, without the script (after one `./setup.sh fetch`):

```
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake   # optional: -DTOOLCHAIN_DIR=<folder> -DKNUTH_KERNEL_STATS=ON
cmake --build build
```

Without Ninja, leave out `-G Ninja` (Make).

**On the board** (the NUCLEO-G431RB on USB): `./setup.sh test` flashes
`build/elevator.elf`, waits for the run and prints PASS or FAIL. `./setup.sh
results` reads the numbers again; the board must still hold that `.elf`, since
GDB takes the variables' addresses from it.

These need OpenOCD and a GDB for ARM. On Linux (Debian, Ubuntu), together
with the compiler, everything in one line:

```
sudo apt install openocd gdb-multiarch gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Instead of `gdb-multiarch`, the `arm-none-eabi-gdb` next to the compiler works
too: `TOOLCHAIN_DIR=<folder with arm-none-eabi-gdb> ./setup.sh test` (CubeIDE
ships one in `plugins/*gnu-tools-for-stm32*/tools/bin`).

**CubeIDE's OpenOCD** instead of a system one (paths under `~/st/STM32CubeIDE_*/plugins`):

```
OPENOCD=.../com.st.stm32cube.ide.mcu.externaltools.openocd.linux64_*/tools/bin/openocd \
OPENOCD_SCRIPTS=.../com.st.stm32cube.ide.mcu.debug.openocd_*/resources/openocd/st_scripts \
STLINK_CFG=stlink-dap.cfg ./setup.sh test
```

`setup.sh` passes `set ENABLE_LOW_POWER 1`: without it, ST's `stm32g4x.cfg`
clears `DBG_SLEEP` in `DBGMCU_CR` on attach, and attaching to the core asleep
in `__WFI` fails with "Examination failed".

## Reference: the same elevator in the CubeIDE project (HAL)

The numbers to compare this build with. Elevator-Primes, a CubeIDE project
with only the kernel and the elevator: 66 users, 1 Knuth unit = 1 ms, 170 MHz,
SysTick off (`HAL_Tick_From_TIM2`), GCC 14.3, `-O3`, on the C++ `-std=c++20
-flto -DNDEBUG`, `POOL_SIZE` 128. Every run: 544 steps, checksum 2487227626, no
refused operations, no dropped trace records.

| | Trace off | Trace on | Old project (link + OLED in main) |
|---|---:|---:|---:|
| Cycles per step | **219** | 518 | — |
| Longest step (cycles) | 665 | 1294 | — |
| Lateness max / average | 6 µs / 286 ns | 7 µs / 408 ns | 6 µs / 327 ns |
| Wake-ups | 473 | 1640 | 473 |
| CPU awake (of 10.36 s) | **1.31 ms** | 30.4 ms | 2.70 ms |
| Idle | 99.99 % | 99.7 % | 99.97 % |

- **Trace off:** the steps take 119 530 cycles (0.70 ms); the other 0.61 ms is
  the wake-ups themselves (interrupt entry and exit, PendSV, one pass of main):
  about 220 cycles (1.3 µs) per wake-up.
- **Trace on** (`TRACE`, lines over LPUART1 DMA at 921 600 baud): +299 cycles
  per step for writing the records in PendSV; about 29 ms in main, mostly
  `snprintf` in `format_trace` (an estimate: ~580 lines, ~8 000 cycles each,
  not yet measured with DWT); +1167 wake-ups, one transfer-complete per line.
- **Old project:** the extra ~1.4 ms awake is main's other work (`Link_Poll`,
  the OLED, the trace poll). Its cycles per step were not recorded in that run.

## Commands

| Command | What it does |
|---|---|
| `./setup.sh` | `fetch`, then `build` |
| `./setup.sh fetch` | CMSIS into `third_party/cmsis` (pinned commits, SHA-256 checked) |
| `./setup.sh build` | CMake configure and build: `build/elevator.elf`, `build/elevator.map` |
| `./setup.sh flash` | build (only what changed), then program the board over the ST-LINK (OpenOCD) |
| `./setup.sh results` | read `elevator_phase`, `elevator_cpp.checksum`, `kernel_stats_level_14`, `elevator_cpp` (with `stack_used`), `is_stack_guard_on`, `stack_guard_fault` from the board (GDB through OpenOCD) |
| `./setup.sh test` | `build`, `flash`, wait 12 s (the run takes 10.4 s), `results`, then PASS or FAIL: steps, checksum, 473 wakes, `idle_us` ≤ `elapsed_us`, the stack guard on and `stack_guard_fault.count` = 0. Wakes and idle catch broken sleep accounting, which steps and checksum don't (the `DBG_SLEEP` misfetch after `__WFI`) |
| `./setup.sh clean` | remove `build/` |

| Setting | Default |
|---|---|
| `TOOLCHAIN_DIR=<path>` | `arm-none-eabi-gcc` from `PATH` |
| `STATS=1` | off; on: `KNUTH_KERNEL_STATS` (the WAIT list's counters: sortins, list and walk lengths) |

Needs: `curl`, `sha256sum`, `cmake` >= 3.20, `arm-none-eabi-gcc` >= 10
(C++20); for `flash`, `results` and `test` also `openocd` and `gdb`
(`arm-none-eabi-gdb` or `gdb-multiarch`). Ninja is used when it is installed.

## The stack guard

`main` calls `stack_guard_init()` first
([stack_guard.c](../../knuth_kernel/port/stm32g4/stack_guard.c)). It paints the
stack with `0xDEADBEEF` and makes the bottom 128 bytes of the 1 KB stack
(`0x20007C00`–`0x20007C7F`, MPU region 0) inaccessible. An overflow becomes a
HardFault, which records the fault in `stack_guard_fault`, instead of silently
overwriting the heap. `stack_used` in `elevator_cpp` is the deepest the stack
went (painted): 228 of 896 bytes in a normal run. How it works:
[SCHEME.md](../../knuth_kernel/port/stm32g4/SCHEME.md#the-stack-guard-stack_guardc).

**Why 128 bytes and not 32.** The first FPU instruction in thread mode sets
`CONTROL.FPCA`. From then on every exception taken from `main` reserves the
extended frame, 104 bytes (`EXC_RETURN` = `0xFFFFFFE9`). No `float` is needed
for that: the compiler uses FPU registers to move memory. Which code does it
first depends on the build:

| Build | Compiler | What sets `FPCA` first |
|---|---|---|
| Bare build | GCC 10.3 | `stack_guard_init()`: the paint loop compiles to `vldr` + `vstmia` (FPU registers). There's no BSP in this build. |
| Elevator-Primes | GCC 14.3 (CubeIDE) | `BSP_LED_Init` and `BSP_PB_Init` (`vldr`/`vstr d7`). Here the same paint loop compiles to `strd r2, r2, [r3]`, plain integer registers. |

So assume the extended frame in every build: a 104-byte frame that starts
above a 128-byte guard can't end below it. The size is `_Stack_Guard_Size` in
[stm32g431rb.ld](stm32g431rb.ld), with `ASSERT`s for its alignment.

## What replaces the CubeMX project

| File | Replaces |
|---|---|
| `src/clock.c` | `SystemClock_Config`: 170 MHz from the HSI in registers, the same steps HAL runs (range 1 boost, 4 flash wait states, PLL /4 x85 /2, the AHB /2 step above 80 MHz) |
| `src/main.c` | the generated `main.c`: `clock_170mhz()`, the priority grouping `HAL_Init` sets, `board_main()` |
| `src/board_main.c` | the same file as in the CubeIDE project: the elevator's start, end, statistics, the sleep loop, `PendSV_Handler` |
| `src/sbrk.c` | `sysmem.c`: the heap for `malloc` (the users' pool) |
| `src/init_fini.c` | `_init`/`_fini`: linked with `-nostartfiles`, so newlib's `crt0` (and the stdio it pulls in) is left out |
| `stm32g431rb.ld` | the generated linker script |
| `CMakeLists.txt`, `cmake/arm-none-eabi.cmake` | `.cproject`: the same flags (`-O3`; on the C++ `-std=c++20 -flto -DNDEBUG -fno-exceptions -fno-rtti`) |
| `third_party/cmsis` (fetched) | `Drivers/`: CMSIS 5.9.0 core headers (ARM) and cmsis-device-g4 v1.2.6 (ST: the device header, `system_stm32g4xx.c`, the startup file), both Apache-2.0 |

The kernel's port (`knuth_kernel/port/stm32g4`) needs only CMSIS; its HAL tick
(`HAL_Tick_From_TIM2`) is built only with `USE_HAL_DRIVER`, so here SysTick is
never started and the CPU wakes only at the kernel's deadlines.

Tested to build with GCC 10.3 (Ubuntu's `arm-none-eabi-gcc`) and GCC 14.3
(CubeIDE's), with no warnings.
