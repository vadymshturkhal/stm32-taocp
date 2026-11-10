# arm-none-eabi.cmake - the toolchain: GCC for Cortex-M4F (STM32G4)
#
# TOOLCHAIN_DIR: the folder with arm-none-eabi-gcc; empty = from PATH.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_DIR "" CACHE PATH "folder with arm-none-eabi-gcc (empty: from PATH)")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES TOOLCHAIN_DIR)
if(TOOLCHAIN_DIR)
    set(prefix "${TOOLCHAIN_DIR}/arm-none-eabi-")
else()
    set(prefix "arm-none-eabi-")
endif()

set(CMAKE_C_COMPILER   "${prefix}gcc")
set(CMAKE_CXX_COMPILER "${prefix}g++")
set(CMAKE_ASM_COMPILER "${prefix}gcc")
set(ARM_SIZE           "${prefix}size")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)   # no startup file yet: test with a library

set(cpu "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${cpu}")
set(CMAKE_CXX_FLAGS_INIT "${cpu}")
set(CMAKE_ASM_FLAGS_INIT "${cpu}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${cpu} --specs=nano.specs --specs=nosys.specs")
