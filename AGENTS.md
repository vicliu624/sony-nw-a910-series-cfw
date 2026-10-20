# Sony NW-A910 Series CFW project rules

Read README.md and docs/BUILD.md first. Project directory: sony-nw-a910-series-cfw. This is the Sony NW-A910 series custom firmware project, not the sibling Island Walkman project. Only NW-A918 has device-test reports; NW-A916/NW-A919 remain unverified.

## Project purpose

- Primary purpose: Simplified Chinese/English localization for Japanese A910 devices and a reusable method for Sony NW firmware research. Other models require independent format, key, ABI and hardware checks.
- Chiikawa artwork and the small program launcher/games are customization demonstrations, not mandatory identity or a promised app platform.
- Before new binary analysis, read docs/TECHNICAL.md, docs/TECHNICAL.zh-CN.md or docs/TECHNICAL.ja.md. They are the current handoff; old research is supporting evidence, not the default workflow.
- CI builds en and zh-CN separately. Both include the customization demonstrations; neither has an on-device language switch. Stock-theme/localization-only profiles remain unimplemented.

## Current implementation

- Preserve Sony 1.03 updater behavior, original kernel/initrd/installation script, nine Home items and OPTION behavior.
- The R14 external-app architecture is the baseline: a small startup WALKMAN/APPS chooser and dynamic APPS/<id>/app.ini discovery. Default enters Sony. No R6/R7 Home hooks, FM enablement or music/game concurrency.
- Apps and ROM/save files belong on visible storage. Tetris uses directions and center only; no pause/save. Color iD is silent, 128x128; down opens its menu.
- Source changes require appropriate offline checks. Never confuse QEMU/capacity results with hardware qualification.

## Build and files

- Supported entry point: scripts/build_release.py on Linux/WSL. VERSION controls SemVer package identity, not Sony's 1.03 version. Tag must equal v<VERSION>.
- Output only under this project: work/ for temporary output, dist/<version>/ for current generated packages. Do not generate deliverables in chat outputs or edit the sibling desktop project as a side effect.
- Old packages, original installers, device backups and recovery evidence are local/ignored; do not commit ROMs, seed saves or personal data. Preserve known-running rollback packages and R12 failure evidence.
- Historical docs are docs/archive/. Superseded tooling and local history are archive/. Do not treat their old paths or instructions as active.
- CI publishes artifacts and draft prereleases only, with ROM tests explicitly excluded. Build/test from clean inputs; no dependency on releases/, archive/ or incidental work files. Cross-toolchain byte reproducibility is not claimed.

## Hardware boundary

- R12 failed installation, rootfs overflow was reproduced offline, device recovery remains unresolved. No new flashing of the unrecovered device.
- User confirmed no USB window during reset. Do not ask for blind repeated reset tests.
- No guessed board pads, shorts, UART wiring or further speculative disassembly. Hardware guidance requires a matching successful recovery account or verified same-board documentation.
- ARM-mode code must use Sony OABI SWI immediates; modern r7+svc0 is incompatible with the original kernel. Preserve sony_oabi.S and its signal-return convention.
