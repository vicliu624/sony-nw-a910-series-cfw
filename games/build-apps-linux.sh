#!/bin/sh
set -eu
cd "$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
out=work/game-apps
mkdir -p "$out"
flags='-std=c99 -Os -march=armv5te -marm -mfloat-abi=soft -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -fomit-frame-pointer -ffunction-sections -fdata-sections -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0'
link='-nostdlib -static -Wl,--gc-sections -Wl,-e,_start -Wl,--build-id=none'
common='games/src/runtime.c games/src/boot_arm.c games/src/sony_oabi.S'
arm-linux-gnueabi-gcc $flags -DA918_EXTERNAL -DA918_LAUNCHER $link games/src/app_launcher.c $common -lgcc -o "$out/a918-boot"
arm-linux-gnueabi-gcc $flags -DA918_EXTERNAL -DA918_STANDALONE -DA918_APP_TETRIS $link games/src/app_tetris.c games/src/blocks.c games/src/standalone_tests.c $common -lgcc -o "$out/tetris"
defs='-DA918_COLOR -DA918_COLOR_PERF -DA918_COLOR_MENU -DA918_EXTERNAL -DA918_STANDALONE -DA918_APP_COLOR -DTC_HOST'
arm-linux-gnueabi-gcc $flags -O2 $defs -Dcpu_reset=a918_color_cpu_reset -Dcpu_step=a918_color_cpu_step -Igames/vendor/color-core -c games/vendor/color-core/tc_all.c -o "$out/core.o"
arm-linux-gnueabi-gcc $flags $defs '-DGAME_DIR="."' $link games/src/app_color.c games/src/color.c games/src/standalone_tests.c $common "$out/core.o" -lgcc -o "$out/tamagotchi-id"
qemu-arm -cpu arm926 "$out/a918-boot" --selftest
qemu-arm -cpu arm926 "$out/tetris" --selftest
for mode in normal crash stall signal save-grace; do qemu-arm -cpu arm926 "$out/a918-boot" --supervisor-test-$mode; done
arm-linux-gnueabi-size "$out/a918-boot" "$out/tetris" "$out/tamagotchi-id"
