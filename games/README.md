# Experimental A910 custom-code examples

This is a demonstration of device customization, not the main project purpose or a general app-platform promise. Localization and the reusable firmware method are documented in [the technical guide](../docs/TECHNICAL.md).

See ../docs/APPS.md for ABI and controls, ../docs/BUILD.md for supported commands. The current build is build-apps-linux.sh; it compiles the launcher, Tetris and color iD as ARMv5/OABI programs and runs ROM-free self-tests with QEMU. It never operates a device.

src/ contains platform, game and shared legacy conditional source. vendor/color-core contains GPL emulator sources and provenance. Old experimental hooks, preview tooling and earlier build entry points are preserved locally in ../archive/games/ and are not supported build commands.
