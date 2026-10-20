# A918 English 1.03 / Chiikawa R6 Game ALPHA1

This is a hardware-test candidate, not a device-verified stable release.
No automatic installation or device writes were performed during its build.

## What is actually integrated

The original Sony Home update entry at VA 0x2a328 branches to a 388-byte ARM
extension in a new read/execute ELF segment at 0xc00000. On any event other than
the selected Home OPTION short activation, the hook restores all registers,
replays the displaced instruction, and continues at 0x2a32c.

OPTION at Sony Home starts the built-in companion that renders Home page 2.
It contains Game and FM Radio (PENDING), making nine original plus two added
functions. The nine original items, their 9x16-byte dispatch table at 0x4634f8,
resources, creation count, layout and target functions are not replaced. The
second page is drawn by the companion; the original Sony grid allocation is
not expanded unsafely from nine to eleven entries. This is not a boot selector
or a replacement for TV. Home virtual key 7/state 4 is inferred from the stock
input accessors/callers (0x76b88, 0x16fd00); the complete Home event route still
needs hardware validation. Raw Linux event codes are independently calibrated.

The PageIn flag at Home+0x658 is deliberately preserved. Disassembly at
0x295b8/0x29624 shows it is not a keyboard-event pending flag.

Game uses an Anoko/あのこ icon, absent from the original nine. Its identifying
horn, pointed ears, beige eyes and striped green tail were checked against
https://chiikawamarket.jp/collections/anoko . Artwork was generated with the
built-in image_gen tool; A1.png was a style reference. The selected master is
assets/chiikawa/masters/A10-Anoko-v2.png; runtime sprite is 72x64 RGB565+alpha.
Mechanical conversion and the final prompt are recorded in assets/chiikawa/game.

## Handoff and recovery

1. The hook uses Sony's libc fork wrapper. The forked child uses only raw
   syscalls to exec /usr/local/bin/a918-games --sony-menu. The parent waits.
2. The companion validates that its direct parent's executable is Gemini;
   validates framebuffer layout; snapshots the screen; validates USB monitoring.
   Unsupervised direct display takeover is refused.
3. A separate rescue process is created before Gemini can be stopped. Then the
   guardian stops only its own verified parent, never a process found by name.
4. A game worker owns a new exclusive input grab. A gate prevents it from
   running until the rescue process has received its PID. Heartbeats detect a
   stalled worker. The inherited Sony input descriptor is never read by us.
5. On exit/crash/timeout/USB transition, the worker is reaped, rescue restores
   the framebuffer and sends SIGCONT to the original player. If the guardian
   itself dies, EOF on its lifetime pipe triggers rescue independently.

USB monitoring uses ONLY 0x80046701 on /dev/usbconn. This is the read/copy-to-user
request used by AscUsbConn_getStatus at 0x1d0f24 and implemented by the included
kernel module at 0x6fc..0x788. No USB-mode-changing ioctl is used. If monitoring
cannot be established, the helper returns before stopping the player. If the
state changes or monitoring fails during play, it terminates the worker and
returns USB handling to Sony. Hardware timing remains untested.

The stock watchdog helper, watchman, reboot script, boot scripts, kernel,
initrd, audio libraries and drivers remain intact. Sony's original watchman
and original libc were run in an isolated QEMU test: its direct child underwent
the six handoff scenarios and completed with EXITED 0. Its production reboot
handler was never executed or changed. No audio processing was added. Pause
music before the first device test; playback behavior after handoff has not
been measured.

## Controls and persistence

First Home OPTION activation may ask for seven keys: LEFT, RIGHT, UP, DOWN,
CENTER, BACK, OPTION. Press and release each once as prompted. It determines
both event codes and press polarity; no A829 keymap is copied. Each prompt has
a timeout and failed calibration returns to Sony.

Page 2: directions select Game/FM; center opens; BACK or OPTION returns to Sony.
Game list: Resume, New Game, Back. Falling blocks: left/right move, up rotates,
down soft drop, center pauses/resumes, OPTION hard drops. BACK saves and returns
to the list; BACK again reaches page 2; BACK once more returns to Sony Home.
FM is only an explanatory status page, with no driver activation or radio audio.

Keys and save state live in /var/a918-games. Saves are CRC32-checked, versioned,
written to a temporary file, fsynced, renamed, and followed by directory fsync.
Corrupt/truncated saves are rejected. Save on pause/BACK/exit, with a 30-second
periodic fallback while playing; unchanged saves are skipped. Known key releases
are drained briefly before returning input ownership. Two-minute input inactivity
returns to the player after saving.

handoff.log is written in the same directory. Failure codes include 77 no FB,
78 unsupported FB, 79 no input, 80 cannot grab, 81 calibration failed, 82 invalid
handoff owner, 83 process/pipe failure, 84 watchdog/interruption, 85 worker killed,
86 rescue failed, 88 USB monitor unavailable, 89 USB transition.

## Verification and remaining device work

- Host ASan/UBSan; ARM926 QEMU; shared WASM and headless browser tests.
- 167 executions of the actual compiled ARM hook in Unicorn, including all
  tested key/state combinations, stock stack/register preservation, missing
  helper, fork/exec failures, interrupted waits and consumed extension event.
- Actual ARM guardian/rescue with normal exit, killed worker, no heartbeat,
  killed guardian, graceful SIGTERM and USB-transition injection. A shared
  fake framebuffer is restored in all six scenarios, including row padding.
- Final UPG independently extracted with Rockbox. All 614 tar entries checked:
  only the original main executable changes and one executable is added.
  All other file content and metadata are identical to pinned R5. English
  wording and both theme DPK files remain byte-identical. Exact ELF modifications
  and preserved ABI/NUMA notes checked. Tests rerun on extracted final payloads.

Not yet proven: device boot/loader compatibility, the original Home key route,
visible framebuffer plane/format, physical key calibration, storage durability
on the device, playback after return, real USB transitions, backlight/power/HOLD
behavior and repeated player/game round trips. Offline tests are not evidence
that these device requirements have passed. Keep the goal open until tested.

## Build and verify

1. powershell -File games/build.ps1
2. node games/test-browser.cjs
3. wsl -d Ubuntu-24.04 -- sh games/test-watchman.sh
4. work/venv/Scripts/python.exe scripts/build-game-firmware.py
5. work/venv/Scripts/python.exe scripts/verify-game-firmware.py
6. python scripts/package-game-firmware.py

Unicorn 2.1.4 is additionally installed in work/venv for the ARM-hook tests.
The package uses the stock Sony updater and stock model/version configuration.
Version remains 1.03. No same-version check bypass is applied.
