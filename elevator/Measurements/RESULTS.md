# Elevator simulation: measured results (Claude helped to generate this summary)

Two separate comparisons live here, because they measure different things:

- **Embedded (STM32G4, DWT cycle counts)** -- hand-tuned ARM Assembly vs GCC-compiled C, same algorithm, same hardware. Answers "how much does hand-written ASM buy you over the compiler."
- **Native (laptop, wall-clock)** -- C vs Python, both running the WAIT-list algorithm natively. Answers "how much does the language/runtime cost."

They are not directly comparable to each other (different hardware, different clocks) -- keep them as two separate stories.

## Embedded: ASM vs GCC (STM32G4, Cortex-M4 @ DWT->CYCCNT, 66 users)

| Build | Cold cycles | Warm cycles |
|---|---|---|
| GCC `-O3` | 90322 | 90379 |
| GCC `-O3 -flto` | 77794 | 77782 |
| Hand-tuned ARM Assembly | 70140 | 70092-70170 |

The ASM port beats plain `-O3` by **~22%** and still beats `-O3 -flto` -- the compiler's most aggressive whole-program optimization mode -- by **~10%**.

The ASM number is the end of a long incremental series (see `measure_elevator_performance.c`'s comment trail in `elevator/C_WAIT_LIST`), starting from an initial port at 88401/88295 cycles and improving in ~25 steps (inlining `AVAIL` push/pop, merging `ASM_HOLD`/`ASM_SORTIN`, caching the WAIT list head in a register across the whole run, instruction-scheduling literal-pool loads ahead of branches, alignment directives on hot loop entries, inlining `DELETEW` into the dispatch loop, ...) -- a ~21% cumulative reduction from the first working ASM build to this one.

One outlier is worth flagging rather than hiding: a single measurement at 69220/69154 cycles was recorded running the ASM build immediately after the GCC build in the same session ("if C version run before" in the source comment) -- likely a flash-prefetch warm-up effect from the prior run, not a property of the ASM code itself. The 70140/70092-70170 figures above are the clean, order-independent numbers.

**Flash size**: measured per-object with `arm-none-eabi-size`/`nm`, summing only the elevator-algorithm objects for each side and excluding what's genuinely shared between them (`values.c`'s RNG -- `ASM_U1` calls `BL values` rather than reimplementing it; `shared_state_init`; both counted once, in neither total).

| Build | C (bytes) | ASM (bytes) | ASM savings |
|---|---|---|---|
| No trace (`TRACE` undefined) | 2526 | 1748 | 778 bytes, **~31%** smaller |
| Traced (`-DTRACE`) | 3326 | 2423 | 903 bytes, **~27%** smaller |

Each total is: language-specific step functions (E1-E9 / ASM_E*, U1-U6 / ASM_U*) + that language's scheduler primitives (`cycle`/`decision`/`hold`/`holdc`/`immed`/`sortin` vs their `ASM_*` equivalents) + that language's own init/run/free glue (`start_elevator_simulation.o` / `start_asm_elevator_simulation.o`). `trace.c` compiles to an empty function when `TRACE` is undefined, hence 0 bytes in the no-trace row.

Also worth noting: enabling `-DTRACE` grows the *whole firmware* from 12252 to 18308 bytes (+49%) -- far more than the trace call sites' own size, because pulling in `vprintf`-style formatting drags in a meaningful chunk of the C library. All figures above are FLASH text size (STM32G431RB: 128KB flash / 32KB RAM, per the linker script) -- 18308 bytes is ~14% of the flash budget.

### History: ASM optimization steps (66 users, DWT->CYCCNT)

Every entry below is a real measurement from `measure_elevator_performance.c`'s comment trail, in order. The first 15 runs predate the habit of labeling what changed between them -- cycles moved in small steps between micro-adjustments not individually recorded. Labels start once each step began being written down deliberately.

| # | Cold cycles | Warm cycles | Change |
|---|---|---|---|
| 1 | 88401 | 88295 | initial ASM port |
| 2 | 86343 | 86230 | -- |
| 3 | 86307 | 86316 | -- |
| 4 | 85488 | 85223 | -- |
| 5 | 84664 | 84665 | -- |
| 6 | 83980 | 83977 | -- |
| 7 | 83755 | 83674 | -- |
| 8 | 83571 | 83605 | -- |
| 9 | 83211 | 83128 | -- |
| 10 | 83070 | 82992 | -- |
| 11 | 83122 | 82973 | -- |
| 12 | 82558 | 82554 | -- |
| 13 | 82606 | 82481 | -- |
| 14 | 82528 | 82485 | -- |
| 15 | 82680 | 82433 | -- |
| 16 | 82504 | 82379 | After `ASM_DELETEW` |
| 17 | 82030 | 81974 | After `ASM_DELETE` |
| 18 | 82776 | 82645 | After `ASM_INSERT` creation |
| 19 | 82117 | 82074 | After `ASM_INSERT` using |
| 20 | 80692 | 80672 | After AVAIL PUSH/POP inlining |
| 21 | 79129 | 79111 | After removing size updating from AVAIL PUSH/POP inlining |
| 22 | 77397-77772 | 77382-77498 | After adding `ASM_IMMED` |
| 23 | 76829 | 76929 | After merging `ASM_HOLD` and `ASM_SORTIN` |
| 24 | 76261 | 76242-76333 | After latency hiding in `ASM_SORTIN` |
| 25 | 75262 | 75232-75233 | After saving WAIT list head to R12 |
| 26 | 74851 | 74813 | After saving `WAIT_LIST.head->right1` to R8 before calling `ASM_CYCLE` |
| 27 | 72679-72700 | 72557-72616 | After `.balign 16` on `ASM_U1` |
| 28 | 72346 | 72275-72306 | After hoisting an `LDR` from `ASM_U2` into `ASM_U1` |
| 29 | 72086 | 72001-72122 | After loading `ASM_E6`/`ASM_E3` targets early, then `MOVS` in `ASM_U2` |
| 30 | 72071-72097 | 71994-72015 | After latency hiding in `ASM_U2_2H_CONTINUE` |
| 31 | 71993 | 71910 | After latency hiding in `ASM_U6_BODY` |
| 32 | 71987 | 71906-72019 | After latency hiding in `ASM_U5` |
| 33 | 71935-71936 | 71852-71895 | After replacing the C `cycle1` call with an inline ASM `STR` in `ASM_E1A` |
| 34 | 71768-71770 | 71687 | After adding `.balign 4` before `ASM_E2_LOWER_LOOP` |
| 35 | 70140 | 70092-70170 | After inlining `ASM_DELETEW` directly into `ASM_CYCLE` |
| 36 | 69220 | 69154 | Same build, run immediately after the GCC build (see outlier note above) |

Run 1 -> run 35: **88401 -> 70140 cold cycles, a 20.7% cumulative reduction** from the first working ASM port to the current one -- entirely from register-allocation, inlining, and instruction-scheduling changes, no algorithmic changes.

## Native: C vs Python (laptop, wall-clock, `TRACE=0` / notrace builds)

Small-to-medium scale, C vs Python head to head (`compare_c_python_no_trace.sh`):

| Users | C (notrace) | Python (`TRACE=0`) |
|---|---|---|
| 4 | ~2ms | ~18-21ms |
| 66 | ~2-3ms | ~20-26ms |
| 1000 | ~2ms | ~61-65ms |
| 10000 | ~4-5ms | ~466-478ms |
| 100000 | ~22ms | ~4306ms |

Python's fixed startup cost (interpreter init, module imports) dominates at small scale, so the ratio understates the real per-event cost gap; it grows sharply from there.

C-only scaling, large scale (`measure_c.sh`):

| Users | notrace |
|---|---|
| 100 | ~2ms |
| 1000 | ~2ms |
| 10000 | ~4ms |
| 100000 | ~19-23ms |
| 1000000 | ~174-183ms |
| 2000000 | ~350-354ms |

Consistently **~0.17µs/user** from 100k to 2M users -- clean linear scaling, no superlinear blowup, confirming the WAIT-list/storage-pool design stays O(1) per event even at large scale. (No equivalent Python run exists yet above 10000 users -- large-scale Python timing is an open item if useful.)

## Sample trace output

Same seed (`values_seed(1)`), 4 users, first 11 lines -- C (`elevator_sim`) and Python (`main.py`) are byte-for-byte identical:

```
TIME STATE FLOOR D1 D2 D3 step action
0000   N     2    0  0  0   U1 User 0 arrives at floor 4, destination is 1, give up 370
0035   U     2    0  0  0   E7 Elevator moving up
0086   U     3    0  0  0   E7 Elevator moving up
0135   U     4    0  0  0   U1 User 1 arrives at floor 4, destination is 2, give up 250
0151   U     4    0  0  0   E2 Elevator stops
0151   N     4    0  0  0   E3 Elevator doors start to open
0171   N     4    1  1  0   U5 User 0 gets in
0196   D     4    1  1  0   U5 User 1 gets in
0196   D     4    1  1  0   E5 Elevator doors flutter
0236   D     4    0  1  1   E5 Elevator doors start to close
```

**ASM**: not yet included here -- the ASM port only runs on the STM32G4 board (no native/laptop build), so a live sample needs a UART capture pasted in from hardware. It has been verified byte-identical to the C/Python trace on-device in an earlier session; once a fresh capture is available, paste it in this section alongside the two above.

## Reproducing

```bash
cd elevator/Measurements
./measure_c.sh "100 1000 10000 100000 1000000" 5       # C only, large scale
./compare_c_python_no_trace.sh "4 66 1000 10000" 5      # C vs Python, small scale
./compare_outputs.sh 66                                  # correctness: diff C vs Python trace output
```

Embedded numbers come from flashing `elevator/C_WAIT_LIST` (GCC) and `elevator/ASM_WAIT_LIST` (hand-tuned) onto an STM32G4 Nucleo board and reading `measure_elevator_performance()`'s DWT cycle counts over a debug session -- see that file's comment trail for the full incremental history.
