# Build and release

Project/repository name: `sony-nw-a910-series-cfw`. The local project directory is `C:/Users/VicLi/Documents/Projects/sony-nw-a910-series-cfw`. Existing `A918-…` package names identify the currently supported device and are retained.

The project centers on Chinese/English localization and reusable firmware research. Theme and program changes are demonstrations. The build supports separate en and zh-CN firmware variants. They include the theme/launcher demonstration; a localization-only or stock-theme profile is not implemented. Read [TECHNICAL](TECHNICAL.md) / [中文](TECHNICAL.zh-CN.md) / [日本語](TECHNICAL.ja.md) before changing resources or targeting another model.

## Supported environment

Ubuntu 24.04, Python 3.12, GNU ARM Linux soft-float cross compiler, QEMU user mode, 7-Zip, e2fsprogs and Crypto++. Windows users can run these commands in WSL from the project directory. Do not run the device boot-dispatch script directly on a computer.

```sh
sudo apt-get update
sudo apt-get install -y build-essential gcc-arm-linux-gnueabi qemu-user e2fsprogs p7zip-full libcrypto++-dev pkg-config zlib1g-dev python3-venv
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements.txt
python scripts/check_project.py
python -m unittest discover -s tests -v
mkdir -p tools/rockbox-upgtools
cp third_party/rockbox-upgtools/* tools/rockbox-upgtools/
make -C tools/rockbox-upgtools
python scripts/build_release.py --language zh-CN
python scripts/build_release.py --language en
```

The Sony installer is downloaded from its original server only if absent and must match SHA-256 `6f745a8ff3c90b7abcd5ea185e6f51e4598492f6de5bdb3256599c6f43e39701`. If that server is unavailable, obtain that exact installer and place it at `firmware/original/NW-A910_V1_03.exe`; the build does not skip verification. No updater executable is run during building.

The five hash-bound resource deltas preserve the R14/R5 demonstration baseline. The selected locale is then rebuilt from translations/missing.tsv using scripts/wording.py, and its selector is regenerated from the original ELF. The launcher and dispatcher are built from current source. No old candidate UPG or neighboring project is used. Resource deltas are maintained inputs, not automatically regenerated artwork; their manifest records source/result hashes.

The build compiles ARMv5 soft-float OABI programs, runs launcher and app tests without ROMs, mocks disk dispatch, replays 28 original-kernel syscall decoder cases, checks all original files/metadata, and fully allocates the rootfs in a regular 48MiB image with Sony's mke2fs. At least 4MiB must remain. Rockbox then independently decrypts all four final components. It never mounts or formats a physical disk. Python optimization is forbidden because the inherited test programs use assertions.

Outputs go only to `dist/<VERSION>/<language>/`, named `A918-<VERSION>-<language>-experimental.zip`. Run language builds sequentially in one checkout; they share intermediate work paths. A second build refuses an existing output/staging directory. To rerun, remove only the selected `dist/<VERSION>/<language>` and `work/ci/package-<language>`, after preserving any wanted result. `work/ci/` and `work/game-apps/` are disposable. Different compiler/zlib versions may produce different firmware bytes; record the output hashes and reports. Byte-identical cross-toolchain builds are not claimed.

## GitHub Actions

`.github/workflows/ci.yml` runs on main/master pushes, pull requests, version tags and manual requests. The en/zh-CN matrix runs in isolated jobs. Every successful language job uploads its installation ZIP, verification JSON and SHA256SUMS as an artifact. ROM execution is explicitly skipped, not reported as passed. Proprietary ROMs/seeds must not be uploaded as CI secrets or committed.

Set VERSION first, then tag the reviewed commit as `v<VERSION>`. The tag must match exactly. Only after both language jobs pass do tag builds create one draft prerelease containing both language packages and language-named verification/checksum files. Alternatively select **Run workflow → draft_release**. A failed build never reaches release creation. Existing release tags are not silently overwritten. Draft releases are intentional while the current firmware has no hardware qualification; publication remains a human decision.

The release package includes the original Sony Windows updater with the generated UPG, a ROM-free APPS tree, corresponding application sources and verification reports. Review Sony/character material redistribution rights before making a draft public. A GitHub source archive is available with each tagged release. The repository currently has no configured remote, so no actual hosted run is implied by local validation.

## Version policy

VERSION uses SemVer for this project's package; the first automated package happens to use the R14 demonstration baseline. A compatible fix increments PATCH; compatible additions increment MINOR; incompatible application ABI changes increment MAJOR. Use prerelease identifiers until hardware qualification exists. Historical R14 is the implementation baseline, not a SemVer number. Sony's updater still targets device version 1.03; this file does not bypass same-version update checks.
