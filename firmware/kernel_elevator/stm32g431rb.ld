/* stm32g431rb.ld - STM32G431RB: 128 KB flash, 32 KB RAM
 *
 * The symbols the startup file (ST's startup_stm32g431xx.s) needs:
 * _estack, _sidata, _sdata, _edata, _sbss, _ebss; and "end" for sbrk.c. */
ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 32K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* the stack grows down from the top */
_Min_Heap_Size = 0x200;                /* checked at link time; the heap may grow past it */
_Min_Stack_Size = 0x400;
/* The MPU no-access region at the stack's bottom (stack_guard.c). At least one
 * exception frame with the FPU context: 26 words = 104 bytes (+4 alignment padding),
 * so a frame stacked just above the guard cannot land below it. A power of 2 >= 32,
 * and the stack's bottom must be aligned to it (an MPU region rule). */
_Stack_Guard_Size = 128;
ASSERT((_Stack_Guard_Size & (_Stack_Guard_Size - 1)) == 0 && _Stack_Guard_Size >= 32,
       "_Stack_Guard_Size: a power of 2, at least 32")
ASSERT(((_estack - _Min_Stack_Size) & (_Stack_Guard_Size - 1)) == 0,
       "the stack's bottom is not aligned to _Stack_Guard_Size")
ASSERT(_Stack_Guard_Size < _Min_Stack_Size, "_Stack_Guard_Size: the guard eats the stack")

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))               /* the vector table first, at 0x08000000 */
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text) *(.text*)
    *(.glue_7) *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata) *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  /* The C++ constructors and destructors, run by __libc_init_array */
  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH

  /* Initialized data: kept in flash, copied to RAM by the startup file */
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data) *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  /* Zeroed by the startup file */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss) *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* Fails the link if the heap's and the stack's minimum do not fit */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
