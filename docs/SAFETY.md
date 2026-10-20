# Installation limits

Current firmware is experimental and offline-tested only. Earlier language/theme and R11 game versions have user-reported device success. R12 exceeded the driver-default rootfs capacity in an offline reproduction and failed installation on the user's A918. Recovery remains unresolved. R12 is prohibited; old packages are historical evidence, not release choices.

The new builder preserves Sony's installer script, kernel and initrd. Sony's installer formats the system partition BEFORE extracting the new rootfs. There is no A/B rollback. A package checksum, small download or successful QEMU test does not prove safe installation, and using the original FWUpdater does not eliminate that risk.

The capacity gate uses a 48MiB partition model derived from the stock driver, original Sony mke2fs, and complete file/block allocation with >=4MiB free. Actual device geometry, NAND behavior, power-loss handling, FAT save durability and USB handoff have not been measured. App watchdogs cannot recover uninterruptible kernel I/O. If three unmount attempts fail, the dispatcher deliberately withholds Sony to avoid simultaneous volume access.

Back up user storage and keep the exact stock updater and known-running packages before any independently chosen hardware test. Never interrupt an active update. No verified forced-USB recovery is provided. Do not install on the currently unrecovered device. Do not guess test pads, shorts or hardware recovery sequences.

The release workflow creates packages and draft prereleases only; it never writes a device. Read archived evidence in archive/r12-recovery-investigation.md and archive/recovery-public-evidence-2026-10-19.md for the detailed history. Archived notes may contain superseded proposals and are not current instructions.
