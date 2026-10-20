# R14 Disk Apps LOCAL1

Local candidate only. No player access or firmware installation was performed. Does not recover an unenumerated failed-update device. Original Sony installation still formats root before extraction; no zero-brick guarantee.

The original English/Chiikawa Sony UI is retained. Boot selects WALKMAN or APPS; default times out to WALKMAN. Apps list is discovered from APPS/<id>/app.ini, not fixed game names. Up to 32 valid flat app directories are sorted by id, with five entries per screen, RESCAN and RETURN TO WALKMAN. Direction keys navigate and center opens. Sony's nine original items and OPTION behavior are unchanged.

Firmware contains only a 27,376-byte application launcher and dispatcher. Tetris and Tamagotchi iD are standalone ARMv5 OABI programs on the data volume. No P1, game executable or ROM is added to the rootfs. App programs are checked for ABI, bounded size, ARM ELF and CRC before being copied to RAM for execution. /tmp must actually be ramfs/tmpfs. Executable storage stays noexec. Apps use their own directory as cwd; iD ROM, reset seed and saves stay there. Runtime CRC is an integrity check, not authentication or a sandbox.

Copy WALKMAN-files/APPS to the root of WALKMAN. Never copy the enclosing WALKMAN-files directory. Alternatively use Island Walkman's Application library with the bundled catalog or select this release's repository folder. The tool installs files only and does not install R14 firmware. R13 cannot run these apps.

APPS/tetris: app.ini + app. Directions move/rotate/drop; center ends the round and returns to APPS. No pause or save.
APPS/tamagotchi-id: app.ini + app + rom.bin + new.sav. LEFT=A, center=B, RIGHT=C; DOWN opens Continue / Save and Exit / New Game / Restore Backup. Native 128x128, silent; performance matches prior interpreter design, no new speed claim. Creates state.sav, state.bak and temporary state.tmp.

R13 saves are not automatically migrated. Back up GAMES/TAMAGOTCHI/ID/state.sav and optionally state.bak, then copy them into APPS/tamagotchi-id after app installation. The ROM/reset seed remain byte-identical to R13.

The publisher repository is catalog.json plus <id>/payload directories. Island Walkman can load additional repositories without rebuilding. Nothing was uploaded or publicly released. Sources are supplied in app-source.zip; color core GPL-3.0 text and provenance are included. ROM and Sony firmware are separate proprietary inputs and are not covered by the emulator license.

Offline checks: 19 ARM/QEMU fixture tests (discovery, staging, corrupt/path/ABI rejection, real iD ROM/save/reset/restore), 14 mock dispatcher/timeout tests, 28 original Sony-kernel syscall decoder comparisons. Rootfs fully allocated using original Sony mke2fs on the driver-default 48MiB model: 5,267,456 bytes free (about 5.02MiB). Actual device geometry and FAT/USB/power-loss behavior are unverified. Stock upgrade script, kernel and initrd, and all unrelated root files/metadata are preserved from R13. R13 and historical candidates remain untouched.

Runtime limits: mount/unmount have userspace timeout attempts, but uninterruptible kernel I/O cannot be made safe by a shell timeout. If unmount fails three times the player UI is deliberately withheld to avoid dual volume access. No independently proven recovery channel exists. Preserve this as a LOCAL candidate until hardware recovery and testing are available.

## Application ABI 1

Apps are static ARMv5 soft-float executables using Sony OABI SWI instructions, invoked as app --games with cwd set to their own app folder. They must own/release display and five-way input without starting Sony concurrently. The inherited A918_APP_HEARTBEAT_FD pipe accepts byte 1 (normal heartbeat within 5 seconds) and byte 2 (saving grace of up to 30 seconds, refreshed while progressing). Startup grace is 30 seconds. The independent firmware launcher supervises the application process group and returns to the APPS list after exit/crash or a heartbeat timeout. The supplied app runtime implements the protocol. This is failure containment for trusted apps, not a security sandbox.
