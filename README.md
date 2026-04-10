# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly

# Implemented algorithms:
- Algorithm P (Print table of 500 primes);
- Algorithm A (Multiply permutations in cycle form);
- Algorithm B (Multiply permutations in cycle form);
- Algorithm I (Inverse permutation in place);
- Algorithm J (Inverse permutation in place);
- Josephus Problem (Generalized);
- Stack;
- Queue;

## How to run:
- Navigate to the specific algorithm's folder;
- Run the C script starting with the word `comparing`;
- Note: The latest ARM Assembly versions have the highest trailing number in their filename (e.g., `asm_get_auxiliary_table4.s` is the fully optimized version).

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
        * GCC -O3 (Clean Code): cycles_cold = [9468-9507], cycles_warm = [9438-9442], size = 288 bytes
        * ARM Assembly: cycles_cold = [6634-6641], cycles_warm = 6600, size = 236 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~2,834 cycles (**~30% time reduction**) in the cold version and by ~2,838 cycles (**~30% time reduction**) in the warm one, with ASM consuming **~18%** less Flash memory
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

    * **Inlined Enqueue/Dequeue (integrated) with ASM Hoisting:**
        * GCC -O3 (Clean Code): cycles_cold = [3957], cycles_warm = [3937], size = 208 bytes
        * ARM Assembly: cycles_cold = [3179], cycles_warm = [3151], size = 192 bytes
        * **Summary:** Hand-tuned ASM outperformed GCC by ~778 cycles (**~19.6% time reduction**) in the cold run and by ~786 cycles (**~19.9% time reduction**) in the warm run, with ASM consuming **~7.6%** less Flash memory
        * **Tricks & Insights:** 
            * **Reduced SRAM traffic:** Hoisted `Front`, `Rear` and `Avail` pointers entirely into registers, bypassing memory wait-states across all iterations
            * **L0 Caching:** Used CPU Registers as Level 0 Cache across all iterations
            * **Redundant Load Elimination:** Carried over `Avail` register state from Enqueue into Dequeue loop, bypassing `LDR R1, [R0, #QUEUE_AVAIL]` entirely
            * **16-bit Narrow Encoding:** Forced all operations into low registers to guarantee 16-bit Thumb encodings and shrinking the binary footprint 
            * **Pipeline Alignment:** Used `.balign 4` to prevent fetch-stalls
            * **Cascade Return Architecture:** Fall-through error handling with unified `free_queue_memory_store_avail` exit minimizing epilogue redundancy
            * **Bump Allocator:** Created and integrated a custom Bump Allocator (`balloc`)
            * **Insight (Flag Trick):** Register R5 pulls double duty — NULL sentinel during Enqueue loop and pessimistic false flag for unified exit. A single `MOVS R5, #1` is the entire success acknowledgement
            * **Insight (Deferred NULL Linkage):** Removed `P->link = NULL` from Enqueue hot loop - only the final rear node requires explicit nulling, deferred to `enqueue_loop_sync`
            * **Insight (Two-Pointer Credit & Avail Linking):** Hand-tuned ASM Queue inline outperforms hand-tuned ASM Stack inline by ~247 cycles. This is achieved via initial Avail List bulk-linking and the Knuth/Torvalds double-pointer trick — inverting the conventional Stack is faster than Queue assumption
</details>
    