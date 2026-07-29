# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly
- This repository contains C and hand-tuned ARM Assembly implementations of TAOCP algorithms on STM32G431RB
- It focuses on reproducible cycle-count benchmarking, code-size trade-offs, and low-level optimization techniques

# Implemented Algorithms and Data Structures:
- Algorithm P (Print table of 500 primes)
- Algorithm A (Multiply permutations in cycle form)
- Algorithm B (Multiply permutations in cycle form)
- Algorithm I (Inverse permutation in place)
- Algorithm J (Inverse permutation in place)
- Josephus Problem (Generalized)
- Stack
- Queue
- Algorithm T(Topological Sort)
- Circular List
- XOR Circular List
- Algorithm A (Addition of polynomials)
- Algorithm M (Multiplication of polynomials)
- Doubly Linked List
- Elevator discrete simulation program (Python async)

## Setup:
- Hardware: STM32G431RB (ARM Cortex-M4)
- Profiler: Internal DWT Cycle Counter

## How to run:
- Navigate to the specific algorithm's folder
- Run the C script starting with the word `comparing`
- Note: The latest ARM Assembly versions have the highest trailing number in their filename (e.g., `asm_get_auxiliary_table4.s` is the fully optimized version)
- for cycles measurements use:
```
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // enable trace
    DWT->CYCCNT = 0;                                // reset counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // enable counter
}
```
and then `DWT_Init();` in `main.c`

## Performance summary GCC -O3 and ARM Assembly:
### All cycle counts measured using the internal DWT cycle counter on the Cortex-M4

<details>
<summary><b>Algorithm P (Print table of 500 primes)</b></summary>

* **Base case = `500 primes`:**
    * GCC -O3: cycles_cold = [120736-120741], cycles_warm = [120712-120713], size = 76 bytes
    * ARM Assembly: cycles_cold = [115679-115712], cycles_warm = [115654-115692], size = 72 bytes
    * **Summary:** Hand-tuned ASM won by ~5,057 cycles (**~4.1% time reduction**) in the cold version and by ~5,058 cycles (**~4.1% time reduction**) in the warm one,    with ARM Assembly consuming **~5.2%** less Flash memory
</details>

<details>
<summary><b>Algorithm A (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * GCC -O3: cycles_cold = [2977-2983], cycles warm = 2917, size = 236 bytes
    * ARM Assembly: cycles_cold = [2613-2618], cycles warm = 2547, size = 268 bytes
    * **Summary:** Hand-tuned ASM won by ~364 cycles (**~12.2% time reduction**) in the cold version and by ~370 cycles (**~12.6% time reduction**) in the warm one, with GCC consuming **~11.9%** less Flash memory due to aggressive compiler inlining
</details>


<details>
<summary><b>Algorithm B (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * **Using parallel arrays as a table:**
        * GCC -O3: cycles_cold = [1968-1970], cycles_warm = 1893, size = 296 bytes
        * ARM Assembly: cycles_cold = [1741-1747], cycles_warm = 1662, size = 322 bytes
        * **Summary:** Hand-tuned ASM won by ~227 cycles (**~11.5% time reduction**) in the cold version and by ~231 cycles (**~12.2% time reduction**) in the warm one, with GCC consuming **~8%** less Flash memory
        * **Some tricks and insights:** The "Silicon Butterfly Effect" & Cache-Line Shields (NOP, .balign), The ABI bypass (Removed C-Returns), Instructions sizing
            as Physical Tetris (16-bit vs 32-bit), The Bare-Metal VLA (Variable Length Array) Hack (stack with BIC instead of malloc), 
            Write-Back Hazard Avoidance (TST instead of AND+CBZ), Pre-increment Pointer Hack (for avoiding late SUB)

    * **Using one array as a table:**
        * GCC -O3: cycles_cold = [947-959], cycles_warm = [887-890], size = 260 bytes
        * ARM Assembly: cycles_cold = [778-784], cycles_warm = 736, size = 224 bytes
        * **Summary:** Hand-tuned ASM won by ~169 cycles (**~17.8% time reduction**) in the cold version and by ~151 cycles (**~17.0% time reduction**) in the warm one, with ARM Assembly consuming **~13.8%** less Flash memory
        * **Some tricks and insights:** Pointer Shifting, Hot Path Isolation & Register Recycling, Free Bounds Checking, 16-bit Thumb-2 instructions instead of standard 32-bit instructions, Branchless Conditional Logic
</details>

<details>
<summary><b>Algorithm I (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [181-189], cycles_warm = 167, size = 100 bytes
    * ARM Assembly: cycles_cold = 151, cycles_warm = 139, size = 50 bytes
    * **Summary:** Hand-tuned ASM won by ~30 cycles (**~16.6% time reduction**) in the cold version and by ~28 (**~16.7% time reduction**) in the warm one, with ARM Assembly consuming **50%** less Flash memory
</details>

<details>
<summary><b>Algorithm J (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [225-226], cycles_warm = 203, size = 84 bytes
    * ARM Assembly: cycles_cold = [201-202], cycles_warm = 186, size = 60 bytes
    * **Summary:** Hand-tuned ASM won by ~24 cycles (**~10.6% time reduction**) in the cold version and by ~17 cycles (**~8.3% time reduction**) in the warm one, with ARM Assembly consuming **~28.5%** less Flash memory
</details>

<details>
<summary><b>Josephus Problem (Generalized)</b></summary>

* **Base case = `mod 33 and participants 10111`:**
    * GCC -O3: cycles_cold = 14006, cycles_warm = 13950, size = 92 bytes
    * ARM Assembly: cycles_cold = 4846, cycles_warm = 4816, size = 114 bytes
    * **Summary:** Hand-tuned ASM won by ~9,160 cycles (**~65.4% time reduction**) in the cold version and by ~9,134 cycles (**~65.4% time reduction**) in the warm one, with GCC consuming **~19.2%** less Flash memory
    * **Some tricks and insights:** ASM custom ceiling
</details>

<details>
<summary><b>Stack (linked allocation)</b></summary>

* **Base case = `128 nodes, 128 Push and 128 Pop using Bump Allocator (balloc)`:**
    * **Translation Unit Boundary Push/Pop (not inline integrated):**
        * GCC -O3 (Clean Code): cold cycles = 8,756 | warm cycles = 8,538 | size = 256 bytes
        * ARM Assembly: cold cycles = 6,016 | warm cycles = 5,958 | size = 220 bytes
        * **Summary:** Hand-tuned ASM won by ~2,740 cycles (**~31.2% time reduction**) in the cold run and by ~2,580 cycles (**~30.2% time reduction**) in the warm run, with ASM consuming **~14%** less Flash memory
        * **Tricks & Insights:** 
            * **Aggressive use of Scratch Registers:** Exploiting the lack of branches to safely place the Stack pointer in a scratch register across 256 loop iterations
            * **Strict AAPCS 8-byte Stack Alignment**
            * **16-bit Thumb-2 Density:** Forced all operations into low registers to shrink the binary footprint
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Cascade Return Architecture:** Fall-through error handling to minimize epilogue redundancy,
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight (Flag usage for error handling):** In Pop function C version used flag instead of Struct with two elements (which is ~100 cycles slower) while ASM simply used R0 and R1 for returning error and info

    * **Inlined Push/Pop (integrated) with ASM Hoisting:**
        * GCC -O3 (Clean Code): cold cycles = 3,863 | warm cycles = 3,792 | size = 200 bytes
        * ARM Assembly: cold cycles = [3,313-3,340] | warm cycles = 3,273 | size = 180 bytes
        * **Summary:** Hand-tuned ASM won by ~550 cycles (**~14.2% time reduction**) in the cold run and by ~519 cycles (**~13.6% time reduction**) in the warm run, with ASM consuming **10%** less Flash memory
        * **Tricks & Insights:** 
            * **Reduced SRAM traffic:** Hoisted `Top` and `Avail` entirely into registers, bypassing memory wait-states
            * **L0 Caching:** Used CPU Registers as Level 0 Cache across loops
            * **Redundant Load Elimination:** Carried over register states between Push/Pop loops to bypass memory reads
            * **16-bit Thumb-2 Density:** Forced all operations into low registers to maximize instruction density and shrink the binary footprint 
            * **Pipeline Alignment:** Applied `.balign 4` to prevent fetch-stalls and eliminate the "Memory Butterfly Effect" (preventing pipeline stalls across hundreds of loop iterations)
            * **Deterministic Waterfall Exit:** Zero-branch success path
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight:** Proved that `STRD` (Double-Word Store) is at least two times slower than two consecutive `STR` instructions in this case

    * **Inlined Push/Pop (integrated) with GCC -O3 (#pragma unroll 4) vs ASM Modulo Variable Expansion (MVE mod 4):**
        * GCC -O3 (Clean Code): cold cycles = [3,422-3,423] | warm cycles = 3,268 | size = 448 bytes
        * ARM Assembly: cold cycles = [2,316-2,344] | warm cycles = 2,239 | size = 372 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~1,106 cycles (**~32.3% time reduction**) in the cold run and by ~1,029 cycles (**~31.4% time reduction**) in the warm run, with ASM consuming **16.9%** less Flash memory
        * **Tricks & Insights:** 
            * **Reduced SRAM traffic:** Hoisted `Top` and `Avail` nodes entirely into registers, bypassing memory wait-states
            * **16-bit Thumb-2 Density:** Forced all operations into low registers to maximize instruction density and shrink the binary footprint 
            * **Pipeline Alignment:** Applie `.balign 4` to prevent fetch-stalls and eliminate the "Memory Butterfly Effect" (preventing pipeline stalls across hundreds of loop iterations)
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Modulo Variable Expansion of factor 4 (MVE mod 4):** Achieved 0-Wait-State unrolling by mathematically weaving registers across iterations
            * **Insight (The Permutation Identity):** The register permutation returns to identity after 4 cycles
            * **Insight (Why GCC loses despite unrolling):** Mechanical unrolling replicates the loop body and cannot eliminate inter-iteration register moves. The permutation identity reduces those moves to zero - a structural property no pragma can express
</details>

<details>
<summary><b>Queue (linked allocation)</b></summary>

* **Base case = `128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)`:**
    * **Translation Unit Boundary Enqueue/Dequeue (not inline integrated):**
        * GCC -O3 (Clean Code): cold cycles = 8,820 | warm cycles = 8,793 | size = 272 bytes
        * ARM Assembly: cold cycles = 6,514 | warm cycles = 6,473 | size = 234 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~2,306 cycles (**~26.1% time reduction**) in the cold run and by ~2,320 cycles (**~26.3% time reduction**) in the warm run, with ASM consuming **~13.9%** less Flash memory
        * **Tricks & Insights:** 
            * **Branchless enqueue (Knuth/Torvalds Trick):** Redefined `rear `as `Node**` — writing `P` into either `queue->front` or the old tail's link field through the same pointer, eliminating the empty-queue branch entirely
            * **Load scheduling/latency hiding:** M4 has 2-cycle load latency. Independent loads are issued early and interleaved with stores so every load is exactly when consumed — zero stall cycles in the hot path
            * **Strict AAPCS 8-byte Stack Alignment**
            * **16-bit Thumb-2 Density:** Forced all operations into low registers to shrink the binary footprint
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Cascade Return Architecture:** Overflow and underflow fall directly into shared cleanup — one BL asm_balloc_free, one return sequence, zero duplicated epilogue code
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight (Flag usage for error handling):** In Dequeue function C version used flag instead of Struct with two elements (which is ~100 cycles slower) while ASM simply used R0 and R1 for returning error and info
            * **Insight (uint32_t for Push/Pop loops):** The fastest way to iterate over max_nodes is to create uint32_t i and iterating from end to start

    * **Inlined Enqueue/Dequeue with ASM Hoisting:**
        * GCC -O3 (Clean Code): cold cycles = 3,957 | warm cycles = 3,937 | size = 208 bytes
        * ARM Assembly: cold cycles = 3,179 | warm cycles = 3,151 | size = 192 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~778 cycles (**~19.6% time reduction**) in the cold run and by ~786 cycles (**~19.9% time reduction**) in the warm run, with ASM consuming **~7.6%** less Flash memory
        * **Tricks & Insights:** 
            * **Reduced SRAM traffic:** Hoisted `Front`, `Rear` and `Avail` pointers entirely into registers, bypassing memory wait-states across all iterations
            * **L0 Caching:** Used CPU Registers as Level 0 Cache across all iterations
            * **Redundant Load Elimination:** Carried over `Avail` register state from Enqueue into Dequeue loop, bypassing `LDR R1, [R0, #QUEUE_AVAIL]` entirely
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Cascade Return Architecture:** Fall-through error handling with unified `free_queue_memory_store_avail` exit minimizing epilogue redundancy
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight (Flag Trick):** Register R5 pulls double duty — NULL sentinel during Enqueue loop and pessimistic false flag for unified exit. Storing 1 to R5 is the entire success acknowledgement
            * **Insight (Deferred NULL Linkage):** Removed `P->link = NULL` from Enqueue hot loop - only the final rear node requires explicit nulling, deferred to `enqueue_loop_sync`
            * **Insight (Two-Pointer Credit & Avail Linking):** Hand-tuned ASM Queue inline outperforms hand-tuned ASM Stack inline by ~2*max_nodes cycles. This is achieved via initial Avail List bulk-linking and the Knuth/Torvalds double-pointer trick — inverting the conventional Stack is faster than Queue assumption

    * **Integrated Enqueue/Dequeue (GCC unroll 4 and MVE mod 4):**
        * GCC -O3 (Clean Code): cold cycles = [2,747-2,835] | warm cycles = [2,653-2,678] | size = 428 bytes
        * ARM Assembly: cold cycles = [2,210-2,237] | warm cycles = 2,120 | size = 360 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~537 cycles (**~19.5% time reduction**) in the cold run and by ~533 cycles (**~20% time reduction**) in the warm run, with ASM consuming **~15.8%** less Flash memory
        * **Tricks & Insights:** 
            * **Reduced SRAM traffic:** Hoisted `Front`, `Rear` and `Avail` pointers entirely into registers, bypassing memory wait-states across all iterations
            * **L0 Caching:** Used CPU Registers as Level 0 Cache across all iterations
            * **Redundant Load Elimination:** Carried over `Avail` register state from Enqueue into Dequeue loop, bypassing `LDR R1, [R0, #QUEUE_AVAIL]` entirely
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Cascade Return Architecture:** Fall-through error handling with unified `free_queue_memory_store_avail` exit minimizing epilogue redundancy
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight (Two-Pointer Credit & Avail Linking):** Hand-tuned `ASM MVE mod 4 Queue` outperforms hand-tuned `ASM MVE mod 4 Stack` by ~max_nodes cycles
            * **Insight (The Permutation Identity):** The register permutation returns to identity after 4 iterations
            * **Insight (Why GCC loses despite unrolling):** The permutation identity trick is mathematical and mechanical unrolling, that replicates the loop body, cannot eliminate inter-iteration register moves
            * **Insight (Physical limits):** Stripped the pipeline down to physical limits, achieving `~4 cycles` per Enqueue and `~5 cycles` per Dequeue
            * **Note (MVE mod 4 naming):** Named MVE mod 4 trick after Witold Lipski, now it's called Lipski trick 
</details>

<details>
<summary><b>Algorithm T (Topological Sort)</b></summary>

* **Base case: `n = 50, input_pairs_len = 139 (Hamiltonian Path with 90 cache-thrashing jump edges)`:**
    * **`Queue Based` with `Linked Memory Allocation`:**
        * GCC -O3: cold cycles = `10494` | warm cycles = `10394` | size = `384` bytes
        * ARM Assembly: cold cycles = `7867` | warm cycles= `7822` | size = `308` bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by `~2,627` cycles (**~25% time reduction**) in the cold run and by `~2,572` cycles (**~24.7% time reduction**) in the warm run, with ASM consuming **~19.7%** less Flash memory
        * **Tricks & Insights:** 
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Memory Allocation:** Allocated `COUNT`, `TOP` and `AVAIL_LIST` memory at once, perfectly interleaving `COUNT` and `TOP` arrays to maximize bus bandwidth during initialization (`STMIA`)
            * **Post-Increment Queuing:** Replaced integer `output` counter with hardware post-increment writeback to auto-advance the `output` pointer in 1 clock cycle
            * **Zero-Flag Branching:** Favored 16-bit `CBZ/CBNZ` over standard `CMP + Branch` to execute conditional jumps natively in the hardware without touching the ALU
            * **Insight (MOV 16-bit instructions):** Difference between `MOV R2, #0 then MOV R9, R2` and `MOV R9, #0`, proxying through low register forces 16-bit encoding, direct high register load requires 32-bit

        * **Additional LeetCode Guys stats with C&GCC -O3:** 
            * **LeetCode Regular:** cold cycles = `47308` | warm cycles = `35891` | size = `772` bytes
            * **LeetCode Hero:** cold cycles = `22431` | warm cycles = `22385` | size = `924` bytes
            * **Summary (Knuth vs LeetCode Regular):** C Algorithm T outperformed LeetCode Regular by `36,814` cycles (**~77.8% time reduction**) in the cold run and by `25,497` cycles (**~71% time reduction**) in the warm run, with Algorithm T consuming **~50.2%** less Flash memory (`384` bytes vs `772` bytes)
            * **Summary (Knuth vs LeetCode Hero):** C Algorithm T outperformed LeetCode Hero by `11,937` cycles (**~53.2% time reduction**) in the cold run and by `11,991` cycles (**~53.5% time reduction**) in the warm run, with Algorithm T consuming **~58.4%** less Flash memory (`384` bytes vs `924` bytes)

    * **`Queue Based` with `Sequential Memory Allocation`:**
        * ARM Assembly:         cold cycles = `5152-5167` | warm cycles= `5106-5109` | size = `280` bytes
        * Clang:                cold cycles = `6735-6760` | warm cycles = `6685-6706` | size = `736` bytes
        * Rust (rustc/LLVM):    cold cycles = `7376-7398` | warm cycles = `7235-7239` | size = `742` bytes
        * GCC:                  cold cycles = `8665-8672` | warm cycles = `8565-8568` | size = `324` bytes

        * **Compiler Configuration Notes:** 
            * GCC: -O3 -mcpu=cortex-m4 -mthumb
            * Clang: -O3 --target=arm-none-eabi -mcpu=cortex-m4 -mthumb
            * Rust (rustc): --target thumbv7em-none-eabi -C opt-level=3 -C target-cpu=cortex-m4

        * **Summary:** 
            * Handwritten ASM: Achieves the fastest execution time and produces the absolute smallest binary footprint
            * Clang (LLVM): Prioritizes raw throughput over space. It trades massive flash memory bloat (nearly 3x larger than the ASM) for speed through aggressive loop unrolling
            * Rust (LLVM): Shares the same massive LLVM footprint as Clang but bleeds ~530 extra cycles
            * GCC: The slowest, it maintained a tight binary size by conservatively refusing to unroll the loops, and paid a heavy price in execution time 

        * **Tricks & Insights:** 
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Memory Allocation:** Allocated `COUNT`, `TOP` and `AVAIL_LIST` memory at once, perfectly interleaving `COUNT` and `TOP` arrays to maximize bus bandwidth during initialization (`STMIA`)
            * **Latency Hiding:** Independent loads issued early and interleaved with ALU operations to absorb M4's 2-cycle load latency across the successor traversal chain
            * **Insight (Step-by-step data usage):**  Replaced bulk `LDM/STM` instructions with targeted single loads `LDR/STR` inside the main loop to prevent bus saturation
            * **Insight (Bounds check performance paradox):**  adding bounds check in C version made GCC code 137 cycles faster by giving aliasing proof

        * **Additional `n = 500, input_pairs_len = 1000` case stats:**
            * ARM Assembly:         cold cycles = `42598` | warm cycles= `42525` | size = `280` bytes
            * Rust (rustc/LLVM):    cold cycles = `45740` | warm cycles = `45574` | size = `742` bytes
            * Clang:                cold cycles = `56560` | warm cycles = `56448` | size = `736` bytes
            * GCC:                  cold cycles = `75760` | warm cycles = `75700` | size = `324` bytes
            * **Short Summary:** Clang beats Rust at `n=50` but loses at `n=500`. GCC is slowest at both but scales most predictably. ASM wins at both
            * **Long Summary:** Compiler scaling behavior flips drastically at scale. Clang beats Rust at a small `N=50` dataset and loses heavily at `N=500`, proving its aggressive loop unrolling (736 bytes) chokes at scale, likely due to I-Cache pressure. Rust scales efficiently due to its strict aliasing guarantees. GCC is the slowest at both but scales predictably. Handwritten ASM is the absolute winner in both cases and its 280-byte footprint easily fits in the cache, allowing it to scale flawlessly without LLVM's bloat
            * **Insight (Crashing Clang):**  Commenting {462,461} pair will crash the Clang, while ASM, Rust and GCC effectively handle that case. Why? Hypothesis: Clang's aggressive -O3 loop unrolling in C likely exploited Undefined Behavior (UB) and stripped a termination safeguard, leading to a HardFault. Rust's strict bounds checking and ASM's explicit hardware checks survived the cycle/disjoint safely

    * **`Stack Based` with `Sequential Memory Allocation`:**
        * ARM Assembly:         cold cycles = `4999-5013` | warm cycles= `4955-4964` | size = `272` bytes
        * Clang:                cold cycles = `5733-5758` | warm cycles = `5562-5564` | size = `812` bytes
        * Rust (rustc/LLVM):    cold cycles = `6546-6551` | warm cycles = `6367-6372` | size = `596` bytes
        * GCC:                  cold cycles = `7200-7206` | warm cycles = `6968-6969` | size = `416` bytes

        * **Compiler Configuration Notes:** 
            * GCC: -O3 -mcpu=cortex-m4 -mthumb
            * Clang: -O3 --target=arm-none-eabi -mcpu=cortex-m4 -mthumb
            * Rust (rustc): --target thumbv7em-none-eabi -C opt-level=3 -C target-cpu=cortex-m4

        * **Tricks & Insights:**
            * **Registers Permutation:** Lipski Trick at T6

        * **Additional `n = 500, input_pairs_len = 1000` case stats:**
            * ARM Assembly:         cold cycles = `41096` | warm cycles= `41024` | size = `274` bytes
            * Clang:                cold cycles = `49494` | warm cycles = `49389` | size = `812` bytes
            * Rust (rustc/LLVM):    cold cycles = `52535` | warm cycles = `52431` | size = `596` bytes
            * GCC:                  cold cycles = `59069` | warm cycles = `58021` | size = `416` bytes

            * **Insight (Crashing Clang):**  Commenting {462,461} pair won't crash the Clang in that case, proved that the crash is structure-specific — Queue's linked traversal creates a code path Clang's optimizer incorrectly eliminates. Stack's sequential traversal doesn't trigger the same Undefined Behavior (UB) exploitation
            * **Insight (Rust (LLVM)):**  Rust Stack based is faster than Rust Queue based in `n = 50, input_pairs_len = 139` case, but it's slower in `n = 500, input_pairs_len = 1000` case
</details>

<details>
<summary><b>Circular List</b></summary>

* **Base case = `Init Avail List with 128 nodes, Insert Left 128 nodes, Pop 128 nodes, Insert Right 128 nodes, Clear`:**
    * **`Translation Unit Boundary (TUB) case`:**
        * ARM Assembly: cold cycles = 9887-9901 | warm cycles = 9812-9814 | size = 406 bytes
        * GCC: cold cycles = 10318-10319 | warm cycles = 10268 | size = 476 bytes
        * Clang (LLVM): cold cycles = 11879-11885 | warm cycles = 11770-11772 | size = 590 bytes
        * **Summary:** 
            * GCC matches hand-tuned ASM across three functions (insert_left, insert_right, pop) by generating gold-standard all-16-bit ASM and using latency hiding heuristics with zero-stack, branchless instructions order according to pure C, the only function where GCC lost is "Create Circular List"
            * Clang outperformed GCC in "Create Circular List" function and loses badly on other functions
        * **Tricks & Insights:**
            * **ABI-Driven Argument Ordering in C:** GCC grasped that signature "uint32_t circular_list_pop(bool* pop_is_success, CircularList* circular_list)" is faster than "uint32_t circular_list_pop(CircularList* circular_list, bool* pop_is_success)", and Clang didn't notice the difference
            * **Manual Register Allocation in C:** GCC grasped that changing instructions order in"circular_list_insert_left" function allow compiler to avoid unnecessary PUSH to the stack, saving ~25% execution time, and Clang didn't notice the difference
            * **Cold Measurement Isolation (Useless):** DSB+ISB+.balign 16 does not fix layout variance when the entire benchmark is one GCC compilation unit — barriers flush pipeline state but cannot change where instructions land in Flash
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Flamboyant Exit:** Used a non-volatile register for storing flag value and applied one branch jump in return phase, avoided redundant copy-paste if-else return block. Implementation of Don't Repeat Yourself (DRY) at the Silicon Level and Single-Entry Single-Exit (SESE) control flow pattern

        * **Additional Info:** 
            In ASM Create Circular List function I used Lipski Trick (Modular Variable Expansion or MVE mod 4) with hoisting, Duff's Device and Instruction Scheduling for handling Address Generation Interlock (AGI) which I called simply "Back in Time". That version has the same speed as Clang and Rust but used completely different approach and takes less memory. 
            Here are stats for Create Circular List function:

            * ARM Assembly: cold cycles = 1458-1465 | warm cycles = 1439-1440 | size = 100 bytes
            * GCC: cold cycles = 1722-1730 | warm cycles = 1690-1691 | size = 112 bytes
            * Clang (LLVM): cold cycles = 1471-1478 | warm cycles = 1440-1445 | size = 244 bytes
            * Rust (rustc/LLVM): cold cycles = 1480-1488 | warm cycles = 1441-1446 | size = 128 bytes

        * **Compiler Configuration Notes:** 
            * GCC: -O3 -mcpu=cortex-m4 -mthumb
            * Clang: -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi
            * Rust (rustc): --target thumbv7em-none-eabi -C opt-level=3 -C target-cpu=cortex-m4

    * **`Inline case`:**
        * ARM Assembly: cold cycles = 4898 | warm cycles = 4827 | size = 374 bytes
        * GCC: cold cycles = 5821 | warm cycles = 5770 | size = 340 bytes
        * Clang: cold cycles = 7243 | warm cycles = 7191 | size = 348 bytes
        * **Summary:** 
            * ASM is ~1.19 times faster than GCC and ~1.47 times faster than Clang
            * ASM is 34 bytes larger than GCC and 26 bytes larger than Clang in Flash
            * GCC is ~1.24 times faster than Clang and takes 4 bytes less in Flash
        * **Tricks & Insights:**
            * **Loop Peeling:** handle first node and then do the loop
            * **PTR and AVAIL caching:** store PTR and AVAIL into registers before loops and synchronize them after the loop, which takes some additional bytes
            * **Cross-Phase Caching:** caching was maintained across phase boundaries (e.g., from the Pop loop directly into the Insert Right loop)
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Flamboyant Exit:** Used a non-volatile register for storing flag value and applied one branch jump in return phase, avoided redundant copy-paste if-else return block. Implementation of Don't Repeat Yourself (DRY) at the Silicon Level and Single-Entry Single-Exit (SESE) control flow pattern
            * **ASM, GCC and Clang don't use any unrolling**
            * **One can shrink ASM binary without losing performance (Hometask)**

        * **Additional nodes=512 results:** 
            * ASM: cold cycles = 19006 | warm cycles = 18938 | size = 374 bytes
            * GCC: cold cycles = 22813 | warm cycles = 22763 | size = 340 bytes
            * Clang: cold cycles = 28459 | warm cycles = 28456 | size = 348 bytes

        * **Additional nodes=512 summary:** 
            * ASM is ~1.2 times faster than GCC and ~1.5 times faster than Clang
            * GCC is ~1.24 times faster than Clang, which is the same as in 128 nodes stats

        * **Compiler Configuration Notes:** 
            * GCC: -O3 -mcpu=cortex-m4 -mthumb
            * Clang: -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi


    * **`Integrate case`:**
        * ARM Assembly: cold cycles = 2865 | warm cycles = 2766 | size = 516 bytes
        * GCC: cold cycles = 3946 | warm cycles = 3886 | size = 396 bytes
        * Clang: cold cycles = 3410 | warm cycles = 3313 | size = 702 bytes
        * **Summary:** 
            * ASM is ~1.405 times (40.5%) faster than GCC and ~1.198 times (19.8%) faster than Clang
            * ASM is 120 bytes (30.3%) larger than GCC and 186 bytes (26.5%) smaller than Clang in Flash
            * Clang is ~1.17 times (17.3%) faster than GCC but 306 bytes (77.3%) larger in Flash
        * **Tricks & Insights:**
            * **Topological Slice:** exploiting the AVAIL_LIST internal structure to reduce O(N) memory writes to O(1), splicing N nodes at once instead of updating LINK of every single node
            * **Loop Peeling:** handle first node and then do the loop
            * **PTR and AVAIL caching:** store PTR and AVAIL into registers before loops and synchronize them after
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrink the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Lipski Trick mod 2:** Registers Permutation Identity which remove MOVS instructions in unrolling
            * **Latency Hiding (Instruction Scheduling):** changed instructions order to avoid pipeline stalls
            * **Removed If-Then-Else blocks in ASM Circular List Pop**

        * **Additional nodes=512 Stats:** 
            * **Results:** 
                * ASM: cold cycles = 10445 | warm cycles = 10349 | size = 516 bytes
                * GCC: cold cycles = 15229 | warm cycles = 15121 | size = 396 bytes
                * Clang: cold cycles = 12914 | warm cycles = 12818 | size = 702 bytes

            * **Ratios:** 
                * GCC / ASM: 15121 / 10349 = 1.461 → ASM 46.1% faster than GCC 
                * Clang / ASM: 12818 / 10349 = 1.239 → ASM 23.9% faster than Clang 
                * GCC / Clang: 15121 / 12818 = 1.180 → Clang 18.0% faster than GCC
            
            * **Per node warm cycles / (512*4):** 
                * ASM:  10349 / 2048 = 5.05 cycles/node
                * GCC:  15121 / 2048 = 7.38 cycles/node
                * Clang: 12818 / 2048 = 6.26 cycles/node

            * **Summary: The performance delta between ASM and both compilers widens with N** 
                * **N=128 ratios:** 
                    * GCC / ASM: 3886 / 2766 = 1.405 (40.5%) 
                    * Clang / ASM: 3313 / 2766 = 1.198 (19.8%)

                * **N=512 ratios:** 
                    * GCC / ASM: 15121 / 10349 = 1.461 (46.1%)
                    * Clang / ASM: 12818 / 10349 = 1.239 (23.9%)

        * **Compiler Configuration Notes:** 
            * GCC: -O3 -mcpu=cortex-m4 -mthumb
            * Clang: -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi
</details>
