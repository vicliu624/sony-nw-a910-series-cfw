# Sony NW-A910 Series Custom Firmware

[English](README.md) · [简体中文](README.zh-CN.md) · [日本語](README.ja.md)

Simplified Chinese and English interfaces for Japanese Sony NW-A910 players, with tools and documentation for developing custom firmware.

| English interface and custom theme | Tetris running on the player |
| :---: | :---: |
| <img src="assets/images/a918-custom-interface.jpg" height="400" alt="English interface and custom theme"> | <img src="assets/images/a918-tetris.png" height="400" alt="Tetris running on the player"> |

Photos from earlier A918 builds. The current release still needs hardware testing.

> **Flashing can cause data loss or leave the player unable to boot. Back up your data first. This firmware is experimental; read the [safety notes](docs/SAFETY.md).**

## About

We started with Sony's original update, unpacked the system, located its interface resources, completed the Chinese and English translations, and rebuilt the firmware. The Chiikawa theme and game launcher are examples of changing the interface and running custom code.

The methods may also help with other Sony NW players. Each model needs its own investigation because its system, keys and hardware may differ.

The original resources contain 11 languages: English, Japanese, French, German, Italian, Spanish, Simplified Chinese, Traditional Chinese, Russian, Korean and Portuguese. Chinese and English gaps have been translated. Language IDs and resource layouts are documented for developers working on the other languages.

## Rockbox and acknowledgments

This project builds on [Rockbox](https://www.rockbox.org/)'s research into Sony's UPG format. We used its [upgtools](https://github.com/Rockbox/rockbox/tree/master/utils/nwztools/upgtools) to search for this firmware's DES-equivalent key, unpack and repack the update, and independently verify modified packages.

The localization, resource changes and custom programs run on Sony's original system; this project does not install the Rockbox operating system. Tool sources and provenance are kept in `third_party/rockbox-upgtools/`.

## Compatibility

The firmware is based on Sony A910 version 1.03. Only NW-A918 has device-test reports; NW-A916 and NW-A919 are untested. Earlier localization and theme builds ran on A918. The current version with the game launcher has not completed hardware testing.

CI produces separate Simplified Chinese (`zh-CN`) and English (`en`) packages. Each uses one language; there is no on-device language switch. Current packages include the example theme and launcher. Game ROMs must be supplied separately.

## Development

The [technical guide](docs/TECHNICAL.md) covers unpacking, language resources, icons and backgrounds, custom programs, capacity checks and repacking. It includes source locations, addresses and known problems.

See the [build instructions](docs/BUILD.md) for dependencies and GitHub Actions, and the [program interface](docs/APPS.md) for starting programs, accessing files and returning to the player.

## Build

After installing dependencies, run these commands in order from the project root:

```sh
python scripts/build_release.py --language zh-CN
python scripts/build_release.py --language en
```

Packages are written to `dist/<VERSION>/<language>/`. They contain Sony's updater, the firmware, example programs, sources and verification reports. Commercial game ROMs and initialization saves are not included.

GitHub Actions checks pushes and pull requests. A `v<version>` tag matching [VERSION](VERSION), or a manual release request, creates a experimental prerelease after the builds pass.

## Layout

- `scripts/`: analysis, build and verification tools.
- `translations/`: Chinese and English translations.
- `assets/patches/`: changes against the original firmware.
- `games/`: custom-program examples.
- `docs/`: documentation; `docs/archive/` keeps historical research.
- `work/`: temporary files; `dist/`: build output. Both are excluded from Git.

[VERSION](VERSION) tracks this project's releases. Sony's device firmware version remains 1.03. See [THIRD_PARTY](THIRD_PARTY.md) for code and artwork notices.
