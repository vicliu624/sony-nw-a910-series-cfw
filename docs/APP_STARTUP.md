# Application startup investigation

Alpha.3 addresses two reproduced code defects. On 2026-10-22 the user tested the local English alpha.3 package on NW-A918 and reported that the app list and installed applications work as expected. The tested ZIP SHA-256 is `3ab65e2555e1c6a958f4fd5ac2747df71865c060dd6a05f2c3e7f937ad498fdb`; its UPG SHA-256 is `1f1e563d8aa6e4041731618c2f600c28c3f4802b9d0d942e07d51ef55c3e7085`. This report does not establish every validation item below, Chinese-build hardware behavior, or byte identity of future CI builds.

## Update flag handling

The alpha.2 dispatcher allowed the chooser only when `nvpflag -x fup` returned `0x00000000`. This rejected `0xFFFFFFFF` before running `--choose`. The regression trace consisted only of `sony`.

Sony's model-listed U-Boot sources define `FLG_BOOT_FWUP = 0x70555766` and `ICX1087_CHK_FWUP()` as equality with that word. `nand_boot_linux()` selects the firmware-update boot arguments for that request; other values take the normal branch unless installation/emergency conditions apply. This source supports the flag rule, but does not establish the installed bootloader's bytes or the affected player's actual NVP value.

REA 6.5.0 with Ghidra 12.1.4 also confirms that the original Gemini function at `0x1a4ffc` opens `/dev/icx1087_nvp/9` and writes the four-byte `0x70555766` request. Original Gemini SHA-256: `a5c94ce492fb7ec9cd7a740d30cb79a1c2b43ee782e2da036a72f57dabdcd177`.

The revised dispatcher requires a successful read of exactly `0x` followed by eight hexadecimal digits. It defers `0x70555766` to Sony. Malformed, empty and failed reads also defer to Sony. It never clears or writes NVP. This removes the zero-only restriction without accepting an invalid read. A passing erased-value test proves this code path, not that the affected device had that value.

Source provenance: [Sony NW-A916/A918/A919 GPL page](https://oss.sony.net/Products/Linux/Audio/NW-A916.html), `u-boot-1.1.1.local.tgz` SHA-256 `682f6ae908e1ebfb93ee89ccb81c57ca6895d57eca6c31a8fd9772ab5171de6f`; `board/mp201xx/icx1087/icx1087.h` and `icx1087_nand.c`.

## App discovery

An empty or absent `APPS` directory must not suppress the startup chooser. App discovery happens only after the user chooses APPS. The empty list still provides RESCAN and RETURN TO WALKMAN.

The old manifest parser rejected CRLF line endings, so an otherwise valid manifest written by a Windows text writer disappeared from the list. The parser now strips one trailing carriage return at the line boundary. It continues rejecting unknown/duplicate fields, path traversal, invalid IDs and incompatible manifests. Staging still verifies program length, CRC32 and the basic ARM ELF header before running the RAM copy.

## Display and USB evidence

Read-only REA/Ghidra analysis inspected the original USB driver and recovered kernel:

- `icx1087_usbconn.ko` SHA-256 `26b57aad0efd8e0fb4653421be4025a497f3e645b9262cb9c0bff250636323e2`: `usbconn_ioctl` handles `0x80046701` by copying a four-byte status to the caller. Its initial status is zero. The launcher does not issue the mode-changing requests `0x40046702` or `0x40046703`.
- Gemini `0x1bc6e0` maps `0x80000` framebuffer bytes, sets timing with `0x80044e04` and `{0,33}`, then configures the plane with `0x40044e02`. `0x1bca08` presents via `0x40044e01` and flips the page.
- Original kernel `qfb_ioctl` and `qfb_mmap` were decompiled as well. The 24-byte plane argument and two-page mapping agree with the launcher. Timing setup and mapping are independent in the inspected paths; their differing order alone was not established as a failure and was not changed.
- Original kernel `load_elf_binary` (`0xc00aa6c8`) and `evdev_ioctl` (`0xc0149d74`) were also analyzed with REA. The latter implements the launcher's `0x40044590` grab/release request. The ELF admission checks permit the current ARM executable headers; this does not establish a complete successful `execve` on hardware.

The recovered ELF is an analysis container for the original kernel, not a flash image. REA's `qfb_open` query returned not-found; no evidence is claimed for a function by that name. Complete local evidence is retained under `work/startup-diagnosis/rea/`, with target hashes in REA records. No inspected target was executed on the player.

## Checks and their limits

`games/test-app-dispatch.py` verifies normal/erased/nonzero update values, malformed/failed reads, USB guards, chooser errors, empty apps, staging failure, crashed apps and unmount-before-Sony behavior, using fake services on ordinary files.

`games/test-launcher-arm.py` executes the actual built ARM chooser and Tetris worker instructions in Unicorn. Only the LCD/input/USB syscalls are simulated. It checks chooser return codes, five-way press/release events, fragmented reads, rendered frames, empty-list return, USB escape and resource cleanup. This does not emulate driver timing or the full boot environment.

`games/check-apps-elf-loader.py` replays the original kernel's ELF admission instructions for all three built programs, stopping at its first program-header read. Allocations use emulator memory, and no real filesystem is accessed. Wrong magic, architecture, program-header size and zero-header negative controls must return ENOEXEC. These 15 checks cover header admission only, not the remainder of process creation, mapping or execution. The build runs them after the original syscall-decoder checks and packages `elf-loader-tests.json`.

`games/test-disk-apps.py` rejects stale app fixtures, discovers apps, verifies RAM staging, exercises both supplied ARM applications and checks watcher failures. Because a host normally lacks an ARM binfmt handler, a fixture-only shell adapter invokes QEMU for the staged ARM program. The adapter is never packaged. This tests staging, cwd, supervision and the app's actual ARM test path; it is not an execution test on Sony's kernel.

With private compatible ROM/seed fixtures, the color test also exercises the actual app UI: A/B/C, Continue, confirmed New Game, confirmed Restore Backup, Save and Exit, and reopen. Test writes stay in disposable fixture directories. CI uses `--without-rom`, excludes these private inputs and reports ROM tests as not run.

## Required device validation

Completion requires observing the startup selector on the affected player with USB disconnected; discovering installed apps; starting and controlling Tetris and color iD; returning to APPS and then Sony; reconnecting USB without concurrent filesystem ownership; and verifying music playback. The desktop data-drive interface cannot read the live Linux NVP/USB status or chooser exit code. Formatting removes installed apps but does not remove the firmware's selector.

The user report supports the startup/list/application fix on this English A918 build. Do not extend it to unobserved USB handoff, music playback, power-loss or durability cases. Keep the Sony updater, kernel, initrd, installation script and nine-item Home behavior unchanged. Do not relax filesystem checks or write NVP to force entry.

## External-storage observation (2026-10-22)

Before application installation, Windows reported the connected WALKMAN volume as requiring repair; read-only CHKDSK found no structural errors. Tetris installed and matched its source hashes. Writing the color ROM encountered I/O errors and disk events 51/153; a subsequent paced attempt completed and all four app files matched their source SHA-256 values. The app was promoted from staging only after verification. This does not prove storage health or resolve the intermittent transfer problem. ROM and initialization saves remain private and excluded from the repository and release.
