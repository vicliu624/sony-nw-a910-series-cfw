# Local history and superseded tooling

Current entry point: ../scripts/build_release.py. Do not run old hook, mount or flashing scripts as the current build.

- scripts/, games/: superseded research/build tools, preserved locally and excluded from Git because they depend on private historical artifacts.
- assets/: original artwork editing inputs and generated theme variants, preserved locally. Current builds use the checked assets/patches deltas.
- local/releases/: older packages including known-running R11, R12 DO-NOT-INSTALL evidence and recovery packages. R14 remains under releases/.
- local/experimental-firmware/: exact historical UPGs, unchanged.
- local/recovery/: preserved device backups, logs, investigations and non-reproducible evidence.

Nothing in this archive is a recommendation to install or perform hardware recovery. Current build never consumes this directory.
