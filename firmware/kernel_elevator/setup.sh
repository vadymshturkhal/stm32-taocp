#!/usr/bin/env bash
# setup.sh - the C++ elevator on the Knuth kernel for a NUCLEO-G431RB, from nothing
#
#   ./setup.sh            fetch what is missing, configure, build -> build/elevator.elf
#   ./setup.sh fetch      only fetch CMSIS (pinned, checked); the kernel and the elevator
#                         are in this checkout (../..)
#   ./setup.sh build      only configure and build
#   ./setup.sh flash      program the board over the ST-LINK (OpenOCD)
#   ./setup.sh results    read the numbers from the running board (GDB + OpenOCD)
#   ./setup.sh test       flash, wait for the run (~10.4 s), read, check 544 / 2487227626
#   ./setup.sh clean      remove build/ (third_party/ stays)
#
# Settings (environment):
#   TOOLCHAIN_DIR=<path>  the folder with arm-none-eabi-gcc (default: from PATH)
#   STATS=1               build with KNUTH_KERNEL_STATS (the WAIT list's counters)
#
# Needs: curl, sha256sum, cmake >= 3.20, arm-none-eabi-gcc >= 10 (C++20);
# for flash/results/test also openocd and gdb (arm-none-eabi-gdb or gdb-multiarch).
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="$HERE/build"
THIRD="$HERE/third_party"
ELF="$BUILD/elevator.elf"

# Pinned versions: CMSIS 5.9.0 (ARM) and cmsis-device-g4 v1.2.6 (ST), both Apache-2.0
CMSIS_CORE_SHA=2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
CMSIS_G4_SHA=25664ddc3a7624ae9627ae8c4c672073dc5b2539
CORE_URL="https://raw.githubusercontent.com/ARM-software/CMSIS_5/$CMSIS_CORE_SHA"
G4_URL="https://raw.githubusercontent.com/STMicroelectronics/cmsis-device-g4/$CMSIS_G4_SHA"

EXPECT_STEPS=544
EXPECT_CHECKSUM=2487227626
OPENOCD_ARGS=(-f interface/stlink.cfg -f target/stm32g4x.cfg)

say() { printf '\n== %s\n' "$*"; }
die() { printf 'setup.sh: %s\n' "$*" >&2; exit 1; }
need() { command -v "$1" >/dev/null || die "needs $1"; }
tool() { if [ -n "${TOOLCHAIN_DIR:-}" ]; then echo "$TOOLCHAIN_DIR/arm-none-eabi-$1"; else echo "arm-none-eabi-$1"; fi; }

get() {   # get <url> <file>: download once
    [ -f "$2" ] && return 0
    mkdir -p "$(dirname "$2")"
    curl -fsSL -o "$2.part" "$1" || die "download failed: $1"
    mv "$2.part" "$2"
}

fetch() {
    need curl; need sha256sum
    say "CMSIS (pinned)"
    local core="$THIRD/cmsis/core" dev="$THIRD/cmsis/device" f
    for f in core_cm4.h cmsis_compiler.h cmsis_gcc.h cmsis_version.h mpu_armv7.h; do
        get "$CORE_URL/CMSIS/Core/Include/$f" "$core/$f"
    done
    get "$CORE_URL/LICENSE.txt" "$core/LICENSE.txt"
    for f in stm32g4xx.h stm32g431xx.h system_stm32g4xx.h; do
        get "$G4_URL/Include/$f" "$dev/$f"
    done
    get "$G4_URL/Source/Templates/system_stm32g4xx.c" "$dev/system_stm32g4xx.c"
    get "$G4_URL/Source/Templates/gcc/startup_stm32g431xx.s" "$dev/startup_stm32g431xx.s"
    get "$G4_URL/LICENSE.md" "$dev/LICENSE.md"
    (cd "$HERE" && sha256sum --quiet -c third_party.sha256) || die "CMSIS checksums do not match"
    echo "ok: checksums match"
}

build() {
    need cmake
    say "configure and build"
    local gen=()
    command -v ninja >/dev/null && gen=(-G Ninja)
    cmake -S "$HERE" -B "$BUILD" "${gen[@]}" \
        -DCMAKE_TOOLCHAIN_FILE="$HERE/cmake/arm-none-eabi.cmake" \
        -DTOOLCHAIN_DIR="${TOOLCHAIN_DIR:-}" \
        -DKNUTH_KERNEL_STATS="$([ "${STATS:-0}" = 1 ] && echo ON || echo OFF)"
    cmake --build "$BUILD"
    echo "ok: $ELF"
}

flash() {
    need openocd
    [ -f "$ELF" ] || die "no $ELF: run ./setup.sh first"
    say "flash"
    openocd "${OPENOCD_ARGS[@]}" -c "program $ELF verify reset exit"
}

gdb_cmd() {
    local g
    for g in "$(tool gdb)" gdb-multiarch; do command -v "$g" >/dev/null && { echo "$g"; return; }; done
    die "needs arm-none-eabi-gdb or gdb-multiarch"
}

results() {
    need openocd
    [ -f "$ELF" ] || die "no $ELF: run ./setup.sh first"
    "$(gdb_cmd)" -q -batch -nx "$ELF" \
        -ex "target extended-remote | openocd -c 'gdb_port pipe; log_output /dev/null' ${OPENOCD_ARGS[*]}" \
        -ex "print elevator_phase" \
        -ex "print elevator_cpp.checksum" \
        -ex "print kernel_stats" \
        -ex "print elevator_cpp" \
        -ex "monitor resume" \
        -ex "detach" 2>/dev/null | grep '^\$'   # resume: detach through the pipe leaves the core halted
}

check() {   # check the output of results
    local out steps checksum done
    out="$(results)"
    echo "$out"
    done="$(sed -n 's/^\$1 = //p' <<<"$out")"         # elevator_phase: 1 = ELEVATOR_PHASE_DONE
    checksum="$(sed -n 's/^\$2 = //p' <<<"$out")"
    steps="$(grep -o 'steps = [0-9]*' <<<"$out" | head -1 | grep -o '[0-9]*$')"
    if [ "$done" = 1 ] && [ "$steps" = "$EXPECT_STEPS" ] && [ "$checksum" = "$EXPECT_CHECKSUM" ]; then
        echo "PASS: $steps steps, checksum $checksum"
    else
        echo "FAIL: done $done, steps $steps (want $EXPECT_STEPS), checksum $checksum (want $EXPECT_CHECKSUM)"
        return 1
    fi
}

case "${1:-all}" in
    all)     fetch; build ;;
    fetch)   fetch ;;
    build)   build ;;
    flash)   flash ;;
    results) results ;;
    test)    flash; say "the run (12 s)"; sleep 12; say "results"; check ;;
    clean)   rm -rf "$BUILD" ;;
    *)       sed -n '2,20p' "$0"; exit 1 ;;
esac
