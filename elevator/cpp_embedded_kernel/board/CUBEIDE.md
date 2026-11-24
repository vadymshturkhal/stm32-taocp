# Building from this checkout in STM32CubeIDE

How to build the Knuth kernel and the C++ elevator in a CubeIDE project **from this
repository**, without copying files into `Core/Src`. CubeIDE links two folders of the
checkout into the project, so there is one copy of the sources: an edit in CubeIDE is
an edit in the repository, and CubeMX's regeneration never touches them. The CubeIDE
project itself (the `.ioc`, `main.c`, the HAL) stays outside the repository.

Done this way in Elevator-Primes, a NUCLEO-G431RB project with only the kernel and the
elevator (STM32CubeIDE, GCC 14.3). Its name appears in the paths below; use yours.

## 1. A path variable for the checkout

Project → Properties → Resource → Linked Resources → **Path Variables → New…**:

| Name | Value |
|---|---|
| `TAOCP` | the checkout, for example `/home/<you>/Programming/stm32-taocp` |

It is stored in the project's `.project`: moving the checkout means changing this one
value.

## 2. Link the two folders

Right-click `Core/Src` → New → Folder → **Advanced » → Link to alternate location
(Linked Folder)**:

| Folder name | Location |
|---|---|
| `knuth_kernel` | `TAOCP/knuth_kernel` |
| `cpp_embedded_kernel` | `TAOCP/elevator/cpp_embedded_kernel` |

If the project has copies of these folders from before, delete them first, or every
file compiles twice.

## 3. Exclude what isn't firmware

The two folders also hold PC programs and another project's `main`. Right-click each
one → Resource Configurations → **Exclude from Build…**, for every configuration:

| Exclude | What it is |
|---|---|
| `knuth_kernel/main.cpp` | the kernel's PC demo |
| `knuth_kernel/tests` | the kernel's PC tests |
| `cpp_embedded_kernel/pc` | the PC elevator's `main` |
| `cpp_embedded_kernel/board/pendsv/main.c` | the old CubeIDE project's `main` |

Left to build: the port's `kernel_port.cpp` and `stack_guard.c`, the elevator's five
`.cpp` files, and `board/pendsv/start_cpp_elevator_simulation.cpp`.

## 4. Include paths

Properties → C/C++ Build → Settings → **MCU/MPU GCC Compiler → Include paths**, and the
same list under **MCU/MPU G++ Compiler**:

```
"${workspace_loc:/Elevator-Primes/Core/Src}"
"${workspace_loc:/Elevator-Primes/Core/Src/knuth_kernel}"
"${workspace_loc:/Elevator-Primes/Core/Src/knuth_kernel/port/stm32g4}"
"${workspace_loc:/Elevator-Primes/Core/Src/cpp_embedded_kernel}"
"${workspace_loc:/Elevator-Primes/Core/Src/cpp_embedded_kernel/board/pendsv}"
```

- `${workspace_loc:…}` resolves a linked folder to its real location in the checkout,
  which is what the compiler needs. A relative path like `../Core/Src/knuth_kernel`
  would point at the project folder, where nothing is.
- `Core/Src` is for the project's `uart_tx.h`, which the elevator's trace includes.
- `board/pendsv` is for `start_cpp_elevator_simulation.h`, which `main.c` includes.

## 5. Compiler and linker settings

| Setting | Where | Value |
|---|---|---|
| C++ standard | G++ Compiler → General | GNU++20 or later |
| C++ flags | G++ Compiler → Miscellaneous → Other flags | `-fno-exceptions -fno-rtti -fno-use-cxa-atexit -DNDEBUG` |
| Optimization, for the repository's reference numbers | G++ Compiler → Optimization; Other flags | `-O3`; `-flto` |
| The same at link time | G++ Linker → Miscellaneous → Other flags | `-O3 -flto`: with LTO the optimizing happens at the link |
| Optional, on the C++ | G++ Compiler → Preprocessor | `TRACE` (the trace over LPUART1 DMA), `KNUTH_KERNEL_STATS` |

**Check that the flags arrive.** A project can hold two configurations with the same
name, and the Properties dialog edits the *active* one, while a launch builds the one
named in it. Set the Debug launch to build the active one: Run → Debug Configurations…
→ the launch → Main → **Build Configuration: Use Active**. After a build, the compile
lines in the console, or `Debug/**/subdir.mk`, show the flags that were used.

## 6. What the project's own code calls

In USER CODE blocks, so CubeMX's regeneration keeps them:

| Where | What |
|---|---|
| `main.c`, includes | `start_cpp_elevator_simulation.h`, `kernel_port.hpp`, `stack_guard.h` |
| `main.c`, `USER CODE BEGIN 1` | `stack_guard_init();`, first, before `HAL_Init()` |
| `main.c`, after the clock | `kernel_port_init();`; after the peripherals `HAL_Tick_From_TIM2();` (SysTick off); then the elevator's setup and `kernel_port_start();` |
| `stm32g4xx_it.c`, `PendSV_Handler`, `USER CODE BEGIN PendSV_IRQn 0` | `kernel_PendSV_Handler();`: kernel level 15 |
| `stm32g4xx_it.c`, `HardFault_Handler`, `USER CODE BEGIN HardFault_IRQn 0` | `__asm volatile("b stack_guard_hard_fault");`: the [stack guard](../../../knuth_kernel/port/stm32g4/SCHEME.md#the-stack-guard-stack_guardc)'s record |
| none | `CORDIC_IRQHandler` (kernel level 14) and `TIM2_IRQHandler` are in `kernel_port.cpp`. Leave TIM2 and the CORDIC unconfigured in the `.ioc`, so CubeMX generates neither |
| the linker script | `_Stack_Guard_Size = 128;` and its three `ASSERT`s, as in [firmware/kernel_elevator/stm32g431rb.ld](../../../firmware/kernel_elevator/stm32g431rb.ld) |

## 7. The same build with CMake (optional)

Elevator-Primes also builds with CMake, from the same checkout. Its `CMakeLists.txt`
reads `TAOCP` from `.project`, so both builds always use the same checkout:

```cmake
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/.project eclipse_project)
string(REGEX MATCH "<name>TAOCP</name>[ \t\r\n]*<value>file:([^<]*)</value>" m "${eclipse_project}")
set(TAOCP_REPO "${CMAKE_MATCH_1}" CACHE PATH "the stm32-taocp checkout")

file(GLOB_RECURSE repo_sources CONFIGURE_DEPENDS
    ${TAOCP_REPO}/knuth_kernel/*.c ${TAOCP_REPO}/knuth_kernel/*.cpp
    ${TAOCP_REPO}/elevator/cpp_embedded_kernel/*.c ${TAOCP_REPO}/elevator/cpp_embedded_kernel/*.cpp)
list(FILTER repo_sources EXCLUDE REGEX "/knuth_kernel/main\\.cpp$")       # the same four as step 3
list(FILTER repo_sources EXCLUDE REGEX "/knuth_kernel/tests/")
list(FILTER repo_sources EXCLUDE REGEX "/cpp_embedded_kernel/pc/")
list(FILTER repo_sources EXCLUDE REGEX "/cpp_embedded_kernel/board/pendsv/main\\.c$")
```

plus the include paths of step 4, with `${TAOCP_REPO}` in place of the linked folders.
Keep its exclusions and CubeIDE's the same.

## 8. Check it

Build, start a Debug session, and stop in `elevator_finish()` after the run (about
10 s): 544 steps, checksum 2487227626, 473 wakes with the trace off, 0 refused
operations. The firmware's [README](../../../firmware/kernel_elevator/README.md) has
the reference numbers.

The linked files are the repository's: commit changes to them in the repository.
