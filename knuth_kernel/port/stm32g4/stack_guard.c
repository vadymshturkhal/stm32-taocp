#include <stdint.h>
#include "stm32g4xx.h"
#include "stack_guard.h"

extern uint32_t _estack[];
extern uint8_t _Min_Stack_Size;   // 0x400 default
extern uint8_t _Stack_Guard_Size; // Address of the guard size
volatile uint32_t is_stack_guard_on = 0;
volatile StackGuardFault stack_guard_fault;

/* Guard starts at 0x20007C00 for 0x400 _Min_Stack_Size */
static uint32_t* stack_bottom(void)
{
    return (uint32_t*)((uintptr_t)_estack - (uintptr_t)&_Min_Stack_Size);
}

static uint32_t guard_bytes(void)
{
    return (uint32_t)(uintptr_t)&_Stack_Guard_Size;
}

void stack_guard_init(void)
{
    uint32_t* const bottom = stack_bottom();

    if (((uintptr_t)bottom & (guard_bytes() - 1u)) != 0u) return;

    uint32_t* p = bottom + guard_bytes() / 4u;
    uint32_t* const end = (uint32_t*)__get_MSP() - 16;

    // Paint a stack
    while (p < end) *p++ = STACK_PAINT;

    // Guard on
    MPU->RNR = 0u;                  // Region Number Register
    MPU->RBAR = (uint32_t)bottom;   // Region Base Addres Register

    // Region Attribute and Size Register
    MPU->RASR = MPU_RASR_XN_Msk     // XN = Execute Never
        | (0u << MPU_RASR_AP_Pos)   // AP = Access Permission
        | ((30u - __CLZ(guard_bytes())) << MPU_RASR_SIZE_Pos)    // Size Field
        | MPU_RASR_ENABLE_Msk;

    // Control Register
    // MPU_CTRL_PRIVDEFENA_Msk = Privileged Default map Enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    __DSB();
    __ISB();

    is_stack_guard_on = 1;
}

/* 
 * How deep did the stack ever go since stack_guard_init()?
 * Can be compared with stack_guard_size() to get the margin.
 * Can be used for early warning, when used > 90% of the size.
 */
uint32_t stack_guard_depth(void)
{
    const uint32_t* p = (const uint32_t*)((uintptr_t)stack_bottom() + guard_bytes());
    while (p < _estack && *p == STACK_PAINT) ++p;
    return (uint32_t)((uintptr_t)_estack - (uintptr_t)p);
}

uint32_t stack_guard_size(void)
{
    return (uint32_t)(uintptr_t)&_Min_Stack_Size - guard_bytes();   /* 1024 - 128 = 896 */
}

/* The C part: runs on the fresh stack, old_msp = the stacked frame,
 * exc_return = the LR the core set on the HardFault's entry 
 */
__attribute__((used, noreturn))
void stack_guard_record(const uint32_t* old_msp, uint32_t exc_return)
{
    const uint32_t cfsr = SCB->CFSR;
    stack_guard_fault.cfsr  = cfsr;
    stack_guard_fault.hfsr  = SCB->HFSR;
    /* The address registers hold an address only while their VALID bit is set */
    stack_guard_fault.mmfar = (cfsr & SCB_CFSR_MMARVALID_Msk) ? SCB->MMFAR : 0u;
    stack_guard_fault.bfar  = (cfsr & SCB_CFSR_BFARVALID_Msk) ? SCB->BFAR : 0u;
    stack_guard_fault.sp    = (uint32_t)old_msp;
    stack_guard_fault.exc_return = exc_return;

    /* The frame was stacked: every word is there */
    if ((cfsr & (SCB_CFSR_MSTKERR_Msk | SCB_CFSR_STKERR_Msk)) == 0u)
    {
        stack_guard_fault.lr = old_msp[5];
        stack_guard_fault.pc = old_msp[6];
    }
    else
    {
        /* The stacking failed: the overflow put the frame (partly) in the guard.
         * The Cortex-M4 stacks PC and xPSR first, then R0-R3, R12, LR. R0, the
         * lowest word, is in the guard: that write faults and the rest is
         * abandoned. So PC is kept when it lies above the guard; LR is never
         * written (measured: a marker there stays) and stays 0. 
         */
        const uintptr_t guard_top = (uintptr_t)stack_bottom() + guard_bytes();
        const uintptr_t pc_at = (uintptr_t)&old_msp[6];
        if (pc_at >= guard_top && pc_at < (uintptr_t)_estack) stack_guard_fault.pc = old_msp[6];
    }

    ++stack_guard_fault.count;
    
    /* Stop only with a debugger */
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) __BKPT(0);

    for(;;) {}
}

__attribute__((naked))  /* Tells GCC and Clang to compile this function without using PUSH/POP */
void stack_guard_hard_fault(void)
{
    __asm volatile(
        "mrs r0, msp            \n"   /* 1. the old SP: where the core stacked the frame */
        "mov r1, lr             \n"   /* 2. EXC_RETURN: the frame's type, the mode, the stack */
        "ldr r2, =_estack       \n"   /* 3. the top of RAM */
        "msr msp, r2            \n"   /* 4. a fresh, empty stack */
        "b   stack_guard_record \n"   /* 5. into C: (old SP in r0, EXC_RETURN in r1) */
        ".ltorg                 \n"); /* 6. the literal pool for "=_estack" */
}

#ifndef USE_HAL_DRIVER
/* Replaces the startup file's weak HardFault_Handler */
void HardFault_Handler(void) __attribute__((alias("stack_guard_hard_fault")));
#endif
