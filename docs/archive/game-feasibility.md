# Game launcher and Tamagotchi research — 2026-10-18

Historical first-pass notes: launcher/control proposals and implementation status
below are superseded by `game-r8-boot-alpha1.md`. Updated Tamagotchi source audit
and ARMv5 synthetic execution results are in `tamagotchi-a918-feasibility.md`.

User request: a Game menu with a built-in list, initially falling-block puzzle; directions plus center play/pause for control. Investigate the currently popular Tamagotchi Paradise custom firmware as another game.

## Direct A918 evidence

- Kernel component contains `Linux version 2.6.10_dev-mp201-20070928A for ICX1101`, GCC 3.4.3 MontaVista.
- `icx1087_key.ko` vermagic includes ARMv5.
- Gemini references /dev/fb/0 and /dev/input/event0. Runtime pixel layout, stride, input event codes and inversion must still be measured on A918.
- Home constructor at VA 0x2835c creates nine 80x72 grid entries (loop comparison to 9 at 0x285d8), starting y28. The 3x3 capacity/navigation is implemented in code, not merely a resource index. A tenth main icon needs layout/navigation changes or a second page.
- /etc/init.d/gemini launches `/usr/local/bin/watchman -s 20 -X /usr/local/bin/reboot.sh $command`. Blind SIGSTOP/kill of Gemini can trigger monitoring/reboot behavior. Need a supervised app transition and recovery path, not just a new icon.
- /contents normally gets unmounted before Sony UI starts. A companion executable/save directory on FAT must account for USB ownership and mount lifecycle.

## Falling-block game

Native ARM Linux implementation avoids emulator overhead. Proposed controls: left/right move, down soft drop, up rotate, center pause/resume; center confirms in launcher; BACK returns to game list. OPTIONS may serve hard drop in a later version. Proposed UI is 10x20 board at 10px/block (100x200), with next piece, score, level and line count in remaining screen area. Start silent, pause/stop music before handoff, and release framebuffer/input cleanly on exit. Native code, watchdog, input exclusivity, USB behavior and recovery remain unimplemented/unverified. No new game firmware built.

Game entry choices: maintain existing functions using an Options > Game entry initially, or implement a second home page. Do not silently delete TV or other original functions to make room.

## Most relevant Paradise project found: Paramix Studio

https://github.com/mameise/Paramix-Studio
https://github.com/mameise/Paramix-Studio/blob/main/emu/README.md
https://github.com/mameise/Paramix-Studio/blob/main/NOTICE.md

Author release post links the same project: https://www.reddit.com/r/TamagotchiParadise/comments/1x55asx/paramix_studio_is_live_mod_your_paradise_rom/

It is a browser ROM modification tool for Paradise. It can add on-device menus and TamaEmu/other emulator payloads or Doom. `emu/src/core` is an S1C33 interpreter/JIT core; `emu/src/emu` implements Paradise-specific display, buttons, sound, flash and state; `src/stub/stub.S` hooks Paradise's existing ROM. This is distinct from emulating Paradise itself.

Build flags in emu/src/emu/build.sh: thumbv7m-none-eabi, cortex-m3, mthumb, freestanding, nostdlib, TC_JIT. JIT emits Thumb-2. A918 ARMv5 cannot execute this payload/JIT output directly. Reuse the portable interpreter/core, replace the host layer with Linux framebuffer/input/file/clock adapters, initially disable JIT and sound, then measure speed on A918. Existing ARMv7-M assembly, flash addresses, launcher stubs and memory maps do not transfer. Do not package the whole Paradise ROM into A918 firmware as native code.

The plain interpreter in emu/src/core/tc_fast.c and original tamaemu project provide alternative starting points. Need inspect TC_HOST/direct-memory assumptions before expecting host compilation.

## Other possibilities and distinctions

- https://github.com/maragotchi/tamaemu : emulator for S1C33 color Tamagotchis (P's, iD L, iD, 4U, etc.), not Paradise. C core is a plausible portability target; screen/input/audio/window frontend must be adapted. User supplies ROM; none downloaded in this research.
- https://github.com/infinition/capybara : actually emulates Paradise's SNC73410 ARMv7-M system. Current Cargo.toml includes eframe/wgpu/egui and desktop/audio dependencies, so the current app cannot run on A918 unchanged. A lean core port could be investigated; interpreter speed and memory are unmeasured. Its display model is 128x128 RGB565. More work/risk than a native game.
- https://github.com/autosam/Tamaweb : JavaScript/browser virtual-pet game. A918 does not currently provide its required browser runtime; cannot run its HTML/JS bundle as a native game. Native reimplementation/port is a separate project.
- https://github.com/manuaguirre9/nw-a820-homebrew : related A829 project with native Linux framebuffer, keys, launcher and games. Useful source evidence for possible integration, not A918 compatibility proof. Its keys.h reports inverted events on A829 (0 pressed, 1 released); do not blindly reuse those values on A918. Its boot code has USB/watchdog/mount recovery handling worth adapting. No code imported into our firmware.

## State

Project identification remains provisional until user provides the specific popular firmware link/name. Read-only source research and repository tree snapshots stored under work/game-research. No ROM/game data fetched; no device actions, code injection, firmware replacement or performance claims made. First implementation milestone should be an A918-specific input/framebuffer probe and a safe launcher round-trip, followed by native falling blocks. Pet emulation is a separate measured feasibility milestone.
