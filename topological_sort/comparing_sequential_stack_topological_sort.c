#include <stdint.h>
#include "main.h"
#include "pairs.h"
#include "test_data.h"

// Prototypes
uint8_t c_algorithm_t_stack_sequential(uint32_t n, Pair* input_pairs, uint32_t input_pairs_len, uint32_t* output);
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);
extern uint8_t clang_stack_sequential_algorithm_t(uint32_t n, Pair* input_pairs, uint32_t input_pairs_len, uint32_t* output);
extern uint8_t asm_algorithm_t_sequential_stack(uint32_t n, Pair* input_pairs, uint32_t input_pairs_len, uint32_t* output);
extern uint8_t rust_asm_stack_sequential_algorithm_t(uint32_t n, Pair* input_pairs, uint32_t input_pairs_len, uint32_t* output);

void comparing_sequential_stack_topological_sort(void) {
    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	// uint32_t output[n]; // would crash the program
	uint32_t* output = asm_balloc(n * sizeof(uint32_t));
	uint32_t* asm_output = asm_balloc(n * sizeof(uint32_t));

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cold cycles = 7198-7206 | warm cycles = 6966-6969 | size = 416 bytes
	start = DWT->CYCCNT;
	uint8_t gcc_topological_status = c_algorithm_t_stack_sequential(n, input_pairs, input_pairs_len, output);
	if (gcc_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	gcc_topological_status = c_algorithm_t_stack_sequential(n, input_pairs, input_pairs_len, output);
	if (gcc_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t gcc_topological_sort_cycles_warm = (end - start) - overhead;

	// Clang -O3 --target=arm-none-eabi -mcpu=cortex-m4 -mthumb: cold cycles = 5733-5758 | warm cycles = 5556-5564 | size = 812 bytes
	start = DWT->CYCCNT;
	uint8_t clang_topological_status = clang_stack_sequential_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (clang_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	clang_topological_status = clang_stack_sequential_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (clang_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t clang_topological_sort_cycles_warm = (end - start) - overhead;

	// ARM Assembly: cold cycles = 4999-5013 | warm cycles = 4955-4964 | size = 272 bytes
	start = DWT->CYCCNT;
	uint8_t asm_topological_status = asm_algorithm_t_sequential_stack(n, input_pairs, input_pairs_len, output);
	if (asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_topological_status = asm_algorithm_t_sequential_stack(n, input_pairs, input_pairs_len, output);
	if (asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t asm_topological_sort_cycles_warm = (end - start) - overhead;

	// Rust: rustc (LLVM backend) --target thumbv7em-none-eabi -C opt-level=3 -C target-cpu=cortex-m4
	// cold cycles = 6546-6551 | warm cycles = 6367-6372 | size = 596 bytes
	start = DWT->CYCCNT;
	uint8_t rust_asm_topological_status = rust_asm_stack_sequential_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (rust_asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t rust_asm_topological_sort_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	rust_asm_topological_status = rust_asm_stack_sequential_algorithm_t(n, input_pairs, input_pairs_len, asm_output);
	if (rust_asm_topological_status == 0) return 0;
	end = DWT->CYCCNT;
	volatile uint32_t rust_asm_topological_sort_cycles_warm = (end - start) - overhead;


	// Free heap memory
	asm_balloc_free(asm_output);
	asm_balloc_free(output);
}
