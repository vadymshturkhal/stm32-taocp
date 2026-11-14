#ifndef STACK_GUARD_H
#define STACK_GUARD_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STACK_PAINT 0xDEADBEEFu 

extern volatile uint32_t is_stack_guard_on;

typedef struct
{
    /* 
     * Configurable Fault Status Register 
     * SCB->CFSR: a kind of fault (MMFSR, BFSR, UFSR bits)
     */
    uint32_t cfsr;

    /* 
     * HardFault Status Register 
     * SCB->HFSR: FORCED = escalated to HardFault
     */
    uint32_t hfsr;

    /* 
     * MemManage Fault Address Register
     * SCB->MMFAR: the address the MPU refused (if CFSR.MMARVALID)
     */
    uint32_t mmfar;
    
    /* 
     * BusFault Address Register 
     * SCB->BFAR: the bus error's address (if CFSR.BFARVALID)
     */ 
    uint32_t bfar;

    /* MSP (Main Stack Pointer) at entry:
     * word 0  R0
     * word 1  R1
     * word 2  R2
     * word 3  R3
     * word 4  R12
     * word 5  LR    Link Register: the return address of the function that was running
     * word 6  PC    Program Counter: the instruction that was executing when the fault hit
     * word 7  xPSR  the status flags and the exception number
     * (with FPU context: S0–S15, FPSCR and one more word follow)
     */
    uint32_t lr;
    uint32_t pc;
    uint32_t sp;        /* MSP when the HardFault was entered */
    uint32_t count;     /* HardFaults since reset */
} StackGuardFault;

extern volatile StackGuardFault stack_guard_fault;

void stack_guard_init(void);
uint32_t stack_guard_depth(void);
uint32_t stack_guard_size(void);
void stack_guard_hard_fault(void);

#ifdef __cplusplus
}
#endif

#endif /* STACK_GUARD_H */