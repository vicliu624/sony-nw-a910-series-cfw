# R13 LOCAL1 safety assessment — 2026-10-18

Verdict: NOT approved for hardware flashing. Offline candidate only. No zero-brick guarantee.

Reviewed UPG SHA-256: 55bfb50e11ccf0921dcaa334d1b77f301ec66a83c7e7eb5fbdfa71277a805d85.

Verified improvements: no P1 or iD ROM in system archive; full rootfs allocation simulation on a 48 MiB partition using original Sony BusyBox leaves 5,210,112 bytes; R12 negative control fails allocation. Actual device geometry was NOT read. Stock installer, kernel and initrd retained. Independent unpack and package manifest checks passed. Reports include host/ARM/kernel syscall/dispatcher tests, not hardware qualification.

Blocking installation risk: stock upgrade script formats /dev/mtdblock/1 before unpacking the rootfs (work/game-r13-final-0.bin lines 23 and 60). No transactional rootfs replacement or automatic rollback. Capacity checks address the reproduced R12 overflow, not power failure, NAND errors or all installer failures. No independently usable rescue path has been established on this device.

Runtime findings:
- games/boot-dispatch-external.sh: after three failed unmounts it exits 91 and deliberately withholds Sony startup. This prevents dual filesystem access but can leave the unit without its player UI. Mount/unmount have no independently enforced timeout; dispatcher lacks signal cleanup traps.
- Update flag guard rejects exact fWUp and empty output, but does not validate other output syntax. Its comment overstates handling of unknown values.
- games/src/color_disk_platform.inc: state.tmp is fsynced, renamed over state.sav, then directory fsync occurs. If this last fsync fails, the function reports failure after replacement. It cannot promise unchanged old state for every failure, or power-loss atomicity on the actual old FAT driver.
- Game code writes ordinary files on the FAT data partition, not raw root/NVP blocks. This reduces direct system-write exposure but does not prove safe FAT/USB handoff or power-loss behavior.

Build hardening: several checks use Python assert and can be disabled by Python -O. Independent verification happens after writing a canonical experimental UPG. Future builds should use explicit errors and staged publish only after verification.

No hardware flash, USB access, code modification, or release ZIP replacement was performed for this assessment. Required before any safety endorsement: fix runtime/build findings; establish a tested independent recovery route; verify real geometry; test actual FAT, USB insertion/removal, watchdog and failure handling on recoverable hardware. R13 remains LOCAL1 and is not a recovery package.
