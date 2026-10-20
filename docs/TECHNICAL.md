# Sony NW custom firmware: technical guide

[English](TECHNICAL.md) · [简体中文](TECHNICAL.zh-CN.md) · [日本語](TECHNICAL.ja.md)

## 1. Scope and evidence

The project centers on Simplified Chinese/English localization of Japanese A910-series firmware. Theme replacement and custom programs demonstrate the degree of customization; Chiikawa and the game launcher are examples, not the product identity. This is a starting point for Sony NW research. It is reasonable to investigate other NW models with this workflow, but not to assume the same format, key, OS, ISA, memory map, resources or recovery path. Some models may require an entirely different approach.

Only NW-A918 has device-test reports. The Sony updater lists A916/A918/A919, but A916/A919 are unverified. Earlier localization/theme builds and some game builds worked according to user feedback. R12 installation failed. Current external-program and newly rebuilt bilingual release variants are offline-tested candidates; those older successes do not qualify them as hardware-tested.

Read this guide before repeating binary analysis. Use current source as authority when it differs from archived notes. `docs/archive/` records findings at different dates, including superseded assumptions. `archive/` and old proprietary packages are local history excluded from Git; a fresh checkout must not depend on them. This guide's commands use project-relative Linux/WSL paths and ordinary files only. Never run the extracted installer or device dispatcher on the host.

## 2. Source map and first commands

| Question | Maintained source / entry |
| --- | --- |
| UPG decode, integrity and root replacement | `scripts/upg.py`: unpack, pack, replace_member |
| Offline evidence export | `scripts/inspect_firmware.py` |
| DWR parsing and missing translations | `scripts/wording.py`: parse_dwr, fill_dwr, complete_pack; `translations/missing.tsv` |
| Checked system/resource differences | `assets/patches/manifest.json` and five XOR deltas |
| Update one reviewed resource delta | `scripts/update_resource_patch.py` (preview unless --apply) |
| Current build and package | `scripts/build_release.py` |
| Full installed allocation | `scripts/check-rootfs-capacity.py` |
| ARM program compiler | `games/build-apps-linux.sh` |
| Native entry, framebuffer, keys | `games/src/boot_arm.c`, `sony_oabi.S` |
| Experimental program discovery / supervisor | `launcher_platform.inc`, `launcher_watchdog.inc`, `games/boot-dispatch-apps.sh` |
| Independent container check | `third_party/rockbox-upgtools/` |
| CI, dependencies and version | `.github/workflows/ci.yml`, `requirements.txt`, `VERSION`, [BUILD](BUILD.md) |

Prepare dependencies using BUILD, compile the vendored Rockbox tool, then:

```sh
python scripts/check_project.py
python -m unittest discover -s tests -v
python scripts/build_release.py --language en
python scripts/build_release.py --language zh-CN
```

Builds emit separate firmware packages, not an on-device language switch. Current examples still include the theme and launcher. There is no build-profile switch to remove them. The two commands are sequential in a local checkout because they share work paths; CI uses isolated matrix jobs. Do not run with Python -O/PYTHONOPTIMIZE: inherited parsers/tests use assertions.

## 3. Original input and Rockbox's contribution

Original URL: `https://walkman.update.sony.net/fw/pc/A910/NW-A910_V1_03.exe`.

| Input | SHA-256 |
| --- | --- |
| Sony EXE, 13,628,920 bytes | `6f745a8ff3c90b7abcd5ea185e6f51e4598492f6de5bdb3256599c6f43e39701` |
| Original NW_A91X.UPG, 13,734,432 bytes | `cfbcc372e2a18f11c12a080c9e5f039333e09175f5db03ac841a4227d3896b32` |

Windows Authenticode was recorded as valid, signer SONY Corporation, for the original EXE. That signature does not authenticate our modified UPG. The builder verifies the EXE hash, extracts it with 7z without running it, and requires the exact 15-file inventory (updater EXE, two DLLs, INI, UPG, ten licenses). Downloads are cached in `firmware/original/`; extraction is `work/ci/updater/`. No old release package is required.

Rockbox had already documented and implemented Sony's UPG container and key-search tools. We reused that work, searched the expected hexadecimal key space for this package, and verified a plausible header and all component boundaries. This was not AI breaking public-key encryption. Known older keys did not decode this particular A910 package. The recovered DES-equivalent ASCII key is `b8442c44` (eight bytes, not four bytes parsed from hex); the ASCII marker is `5dfeff3c`. DES parity permits equivalent keys. Rockbox's derived KAS is `ba49adfd57caba10b771b9bef805b2f9`.

An unmodified decode/re-encode roundtrip matched the original UPG SHA-256 exactly; independent Rockbox repacking did too. Current builds independently extract the final package with Rockbox and compare all four components byte-for-byte. This establishes container handling, not installation safety. Vendored tool source provenance does not record its original upstream commit; do not invent one.

## 4. UPG format and component identity

All integer fields below are little-endian. DES mode is ECB with 8-byte blocks.

| Absolute file region | Meaning |
| --- | --- |
| 0..15 | Raw MD5 digest of bytes 16..EOF |
| 16..63, encrypted | 48-byte header for this four-component format |
| Decrypted header +0 | 8-byte ASCII marker `5dfeff3c` |
| +8, +12 | u32 component count=4; u32 reserved=0 |
| +16+8*i | u32 absolute component offset; u32 unpadded size |
| Component offset | DES-encrypted bytes, size rounded up to a multiple of 8 |

Offsets are absolute from the start of the UPG. Validate alignment, ordering, bounds, marker and MD5, not just readable strings. MD5 and the marker are integrity/format fields, not cryptographic publisher signatures.

| Index | Original offset | Original size | Identity established later in the investigation |
| --- | ---: | ---: | --- |
| 0 | 64 | 1,578 | Upgrade shell script |
| 1 | 1,648 | 2,523,136 | Linux kernel image used in syscall-decoder replay |
| 2 | 2,524,784 | 786,432 | ext2 initrd image |
| 3 | 3,311,216 | 10,423,215 | gzip-compressed tar rootfs |

The current builder changes only component 3. pack() preserves the first three encrypted components, updates component-3 length in the encrypted header and recalculates MD5. It is intentionally an A910 helper, not a generic Sony format library.

Export reproducible evidence after a build has prepared the original input:

```sh
python scripts/inspect_firmware.py --upg work/ci/updater/NW_A91X.UPG --output work/inspect-original
```

Choose a new output directory. It exports four components, fixed-name key system files, 12 wording sections and inspection.json. It does not extract arbitrary tar paths or execute ARM code. `component-0.bin` is text to inspect, not a host script to run.

## 5. Rootfs, application and exact language patch

The original archive has 613 entries. Its main UI is `/usr/local/bin/gemini-ICX1101` (ARM Linux ELF, 5,691,632 bytes). Wording lives at `/usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk`; texture packs are under `.../dui_tex/`. Fonts include `dui_font.dpk` and `ICX1101_dui_font.dpk`; do not delete fonts merely because the menus use English—song metadata can still be Japanese/Chinese.

The wording reader loads the configured language and chooses the corresponding DWR, then resolves a text ID. In this exact original executable:

| Item | Value |
| --- | --- |
| Instruction VA | `0x96294` |
| ELF file offset | `0x8e294` |
| Original bytes / instruction | `14 30 90 e5` / `ldr r3, [r0, #0x14]` |
| English | `00 30 a0 e3` / `mov r3, #0` |
| Simplified Chinese | `06 30 a0 e3` / `mov r3, #6` |

This changes one four-byte ARM instruction without moving code. It forces the wording selector only; it does not add a Settings language menu, rewrite destination/NVP or localize separate boot/update screens. VA is not file offset. For a different ELF, find its PT_LOAD segment and compute `p_offset + VA - p_vaddr`, within file-backed bytes; rediscover the instruction and assert original bytes. Never apply this offset to another version/model.

## 6. All 11 language resources

The wording DPK has one common section plus these language sections. DPK section index is language ID+1, not language ID.

| Language ID | DPK section | Language |
| ---: | ---: | --- |
| 0 | 1 | English |
| 1 | 2 | Japanese |
| 2 | 3 | French |
| 3 | 4 | German |
| 4 | 5 | Italian |
| 5 | 6 | Spanish |
| 6 | 7 | Simplified Chinese |
| 7 | 8 | Traditional Chinese |
| 8 | 9 | Russian |
| 9 | 10 | Korean |
| 10 | 11 | Portuguese |

Each language has 826 records. The common section has 34 records; the observed reader handles shared IDs 826..859 without the language selector. Resource presence does not prove translation completeness, glyph coverage, reachable pages or device support. Japanese has 648 nonempty records and 178 empty/reserved ones. Original English has 360 nonempty records (288 gaps vs Japanese); original Simplified Chinese has 359 (289 gaps). Other languages need their own comparison—do not reuse those counts.

For French, German or another locale: compare its nonempty IDs against Japanese, preserve original translations, fill missing IDs, verify fonts/layout, and select that locale at the proven reader. Do not enable a locale and call the result complete without checking blank resources. Do not describe eleven embedded resources as eleven completed, tested language packages.

The current CLI accepts only en and zh-CN. Adding another language requires a TSV column, language/section mapping, an appropriate word-wrapping rule, its measured missing-record count, a selector ID, and final-artifact tests. In particular, the non-English branch of wrap_text is CJK-oriented; do not use it unchanged for French/German word wrapping. Keep the existing en/zh-CN fixtures as regressions.

## 7. DPK/DWR encoding and concise translation

Wording DPK header is `<4I`: payload byte count, section count, two reserved zeros. Each 16-byte index entry is `<QQ`: key and offset. Sections are sliced from offset to next offset; rebuilding a section changes subsequent offsets and the payload count. Preserve keys and unrelated sections.

A DWR header is `<4I`: locale, record count, byte offset of line-length pool, byte offset of UTF-16 pool. Each record is `<bbHHH` (8 bytes): signed line count, signed secondary field, u16 length-pool index, u16 text-pool index, u16 metadata. The pools' indices count 16-bit units, not bytes. Each line length includes its terminating UTF-16 NUL. Validate all ranges and terminators. The secondary field participates in dynamic insertion behavior; copying only visible text loses structure.

`parse_dwr()` recovers records. `fill_dwr()` only fills originally blank target records when Japanese is nonempty, retains existing target metadata, copies the Japanese secondary field for missing entries, and appends new lengths/text rather than overwriting the following string. Pools/sections are aligned to 16 bytes; indices must fit u16. Keep dynamic content positions and blank lines. Preserve all other language sections byte-for-byte.

Translations are `translations/missing.tsv`, columns id/english/simplified_chinese; literal \n denotes a new line. Newly inserted text is wrapped to roughly 20 display-width units (ASCII=1, wide CJK=2); its number of lines must not exceed Japanese. This is a conservative heuristic, not a pixel fit guarantee. The utility generates either resource from the original UPG:

```sh
python scripts/wording.py --upg work/ci/updater/NW_A91X.UPG --language en --output work/localization/en.dpk
python scripts/wording.py --upg work/ci/updater/NW_A91X.UPG --language zh-CN --output work/localization/zh-CN.dpk
```

Current CI rebuilds the selected language from TSV and verifies 288/289 filled records, zero missing vs Japanese and preservation of existing records. It also patches the selector to 0/6. These are two separate firmware variants; the demonstration launcher text remains English.

## 8. UI textures and selection animation

Texture DPKs use the same 16-byte header shape; their entries can be read as `<4I`: key, category, offset, reserved. Packs can contain nested packs. Recognize a pack by payload/count/bounds and increasing offsets, not by extension alone. Observed name hash: start at zero, `h=(37*h+byte)&0xffffffff` for each filename byte, then `h|0x80000000`. Preserve category/reserved fields and nesting.

For the modified format-5 DTX leaves: `<HHI` gives width, height, format at offset 0; the header is 16 bytes, followed by little-endian RGB565 pixels. Encode `((R>>3)<<11)|((G>>2)<<5)|(B>>3)`; expected length is `16+2*width*height`. This applies to the observed leaves, not every Sony texture format. Start with same-size replacements so DTX headers, parent pack offsets and total lengths can stay unchanged.

Home stems: i_shuf / oneseg / search / photo / music / video / setup / rec / now. Files include `home_<stem>.dtx`, `_hot.dtx` and numbered frames. There are multiple states/animation frames, not just two universal icons. Inspect all references and frames. A normal/selected pose can be illustrated across the animation; an attractive static preview does not establish the actual state machine.

The proven Home geometry is 240×320 overall: header y=0..27; nine 80×72 grid tiles from y=28 through 243; label footer 240×44 at y=244..287; status strip 240×32 at y=288..319. Some texture leaves are 80×70—preserve each leaf's own dimensions. The home footer leaf key is 0xe60ccd8f under parent 0xaa48c2d0, in ICX1101_home_anim.dpk. The observed application loads it in texture slot 21. The status background is shared with other pages.

Why the first background looked incomplete: grid, footer and status were separate assets. Slice one continuous canvas at y=28/244/288/320, replace each region, and check all pages sharing the status asset. Native white labels need adequate contrast. A generic 240×320 image elsewhere in the pack may be a power-off/loader graphic, not a Home wallpaper. Chiikawa is only the worked example; none of the format rules requires its artwork.

## 9. Maintaining a reviewed resource change

Current source deltas in assets/patches cover exactly five original files: main Gemini, wording DPK, two texture DPKs and `/etc/init.d/gemini`. Each manifest entry records original/result/delta SHA-256 and result length. Inflate the delta, XOR against the original padded with zeros, then truncate to result length. The data is a maintained input, not a fresh editable-art rebuild in CI.

For a reviewed texture replacement:

```sh
python scripts/update_resource_patch.py --original-upg work/ci/updater/NW_A91X.UPG --path ./usr/local/share/gemini/res/dui_tex/dui_tex.dpk --replacement work/my-theme/dui_tex.dpk
# After checking the preview, repeat with --apply to update that source delta.
```

This tool updates one existing delta, not a device. It does not prove that the replacement's texture semantics are valid. Inspect the manifest/source diff, decode and compare the changed leaves, rebuild, inspect the final archive and run the capacity check. To add new system paths or support another format, explicitly update the builder's allowlist and verification; do not weaken hashes to make an unknown input pass.

Language resources and selector are generated per language after the baseline deltas. Editing their baseline deltas alone will not override `complete_pack()` and the selector logic. Change translations/wording code or the selector implementation instead. A stock-theme/no-launcher profile is possible development work, not a currently implemented CLI flag.

## 10. Experimental custom-code path

The rootfs example changes `/etc/init.d/gemini` to run `/usr/local/bin/a918-boot-dispatch` before Sony, guarded against recursion with A918_BOOT_BYPASS. The dispatcher is `games/boot-dispatch-apps.sh`; the launcher is a918-boot. Existing emergency and Sony paths are preserved. Original nine-item Home and OPTION handlers are not patched. R6/R7 hooks and page overlays caused freezes and were abandoned; do not reintroduce them as a shortcut.

Only explicitly cleared `nvpflag -x fup` output `0x00000000` permits the chooser. Unknown/pending update and USB-present/unreadable status bypass to Sony. Reading the flag is not permission to write NVP. WALKMAN timeout/default does not mount user data. Explicit APPS selection checks existing mounts and `/tmp` ramfs/tmpfs, mounts `/dev/mtdblock/2` privately at `/tmp/a918-game-disk` with rw,nosuid,nodev,noexec, and checks USB again.

The list reads APPS/<id>/app.ini. Programs pass manifest length/CRC and basic ARM ELF checks, then copy to `/tmp/a918-app-runtime/program`. Working directory is their own storage folder. A process-group heartbeat supervisor contains ordinary crashes/stalls. Returning to Sony requires unmounting; three failures exit 91 and withhold Sony to prevent dual access. Shell timeout cannot recover uninterruptible kernel I/O. No security sandbox exists. See [APPS](APPS.md) and the actual dispatcher for the exact contract.

Programs/ROM/saves stay on user storage to prevent rootfs growth and avoid reflashing for program updates. The iD example reads a 4MiB ROM into RAM, checks its CRC and maintains changes in a save, not the original rom.bin. Saving uses temporary file, fsync, rename and directory fsync; final directory-sync failure can occur after replacement. FAT power-loss durability is unverified; a backup is not created on every save. Do not promise lossless saves or immunity from system hangs.

## 11. CPU, syscall ABI, display and input

The target programs compile `-march=armv5te -marm -mfloat-abi=soft`, static/freestanding, with `_start`; they do not depend on modern glibc. The kernel is Sony's old 2.6.10 ARM environment. ARM-mode `svc 0` with syscall number in r7 is not accepted as modern EABI Linux callers expect. Use immediate `SWI 0x900000+nr`. sony_oabi.S preserves AAPCS C argument/callee-save rules and dispatches through 288 SWI stubs; this is a syscall adaptation, not a claim that all C data layouts are historical OABI. Signal return also needs the correct immediate.

`games/check-apps-kernel-abi.py` replays the original kernel decoder in Unicorn: vector_swi=0xc0022ae0, sys_call_table=0xc0022ba4, arm_syscall=0xc0028388. It skips unrelated CP15 setup and stops at resolved handlers; no hardware syscalls execute. 28 numbers are checked. QEMU-user success alone did not catch this incompatibility.

boot_arm.c opens `/dev/fb/0`, `/dev/input/event0` and `/dev/usbconn`. QFB setup ioctl 0x80044e04 with {0,33} follows stock Gemini and changes timing; despite direction bits, do not describe it as a harmless read. Map 0x80000 bytes, present RGB565 via 0x40044e01 with {1,0x800,255,page,0,0}. USB status uses 0x80046701; failure returns toward Sony. Input EVIOCGRAB is 0x40044590. Observed raw key codes: 0=center, 1=right, 2=left, 3=up, 4=down. Process press and release phases; release grabs/mappings/descriptors before handoff. All are model-specific findings, not portable ioctls.

An ARM CPU is not automatically compatible with every ARM binary or Thumb-2 JIT. The iD interpreter is used on this ARMv5 target; upstream Cortex-M3 code generation cannot simply execute here. Do not select a new host on MHz alone.

## 12. R12 failure and installed-size gate

Original component 0 formats `/dev/mtdblock/1` with `mkfs.ext2 -b 4096`, mounts it and pipes component 3 into tar. Formatting precedes extraction. There is no A/B transactional rollback. Keep this original script/kernel/initrd unchanged; changing host FWUpdater UI does not remove the device-side hazard.

The original NAND driver's default root partition is 0x03000000=48MiB. Actual device geometry has not been read. Using original Sony BusyBox mke2fs 1.37 produced 12,288 blocks with 11,895 free before file allocation. Full non-sparse allocation needs 11,704 blocks for R11 but 11,921 for R12—at least 26 blocks/106,496 bytes over the model. This reproduces a concrete capacity failure and matches the symptoms; without device logs it is not proof of the only hardware failure cause. R12 is prohibited.

Current gate creates only a bounded regular image under work/, runs Sony mke2fs through QEMU, allocates all file blocks and indirect blocks with debugfs (including zero data), directories and links, and requires >=4MiB free. It never mounts a disk. Do not substitute gzip size, sum(file sizes), sparse-copy success or modern default mkfs output.

Original model free space: 5,312,512 bytes. R14 English model: 5,267,456 bytes; +45,056 bytes/44KiB allocated vs stock. These are model results, not live df or NAND health. New variants must use their own capacity reports. External storage solves system-package growth, not write failures, power loss or unproven recovery.

## 13. Build gates, release and what tests prove

Order: verify original EXE → extract exact inventory → compile → ARM tests → mocked dispatcher → original-kernel replay → apply five deltas → generate selected locale → verify unchanged tar files/metadata → allocate capacity image → pack UPG → independently decode with Rockbox → build ZIP/checksums. The two added rootfs files make 615 entries. Original file order, owner/group, permissions, times, links and other checked metadata are preserved. Outputs are published only after checks.

Current public tests: three container regressions, five wording preservation/bounds regressions, 17 ROM-free ARM fixture cases, 14 mocked dispatcher cases, 28 original-kernel decoder cases, plus compiler-script selftests. ROM-based boot/save/reset tests need private inputs and are explicitly skipped in public CI, not counted as passes. Hardware USB handoff, flash reliability, suspend/power loss, NAND/FAT behavior and audio quality have not been proved by these tests. Preserving audio libraries/drivers reduces change scope, not a measurement of unchanged sound.

Use [BUILD](BUILD.md) for dependencies, paths and release commands. VERSION is package SemVer; device Sony version stays 1.03. CI matrix produces en and zh-CN artifacts; only if all builds pass does a matching tag create a draft prerelease. No flashing, NVP writes or physical-device access occurs in CI. The current local repository has no configured remote; a local test is not a hosted workflow result.

## 14. Recovery boundaries and rejected assumptions

The affected A918 repeatedly retried R12 and no USB window was observed. No verified force-USB key sequence or same-failure recovery path is established. The normal rootfs emergency USB path cannot help if that rootfs is unavailable. The stock commented play-key rescue example is disabled, and special factory-key decoding in the main UI does not prove it works during updating. No confirmed UART pads, voltage or accessible shell have been established. Do not recommend guessed shorts, another model's sequence, repeated blind resets or boot-flag edits without a communication path and backup.

Keep known-running R11, original firmware and failure evidence. Historical investigations are [R12](archive/r12-recovery-investigation.md), [public recovery evidence](archive/recovery-public-evidence-2026-10-19.md), [allocation comparison](archive/rootfs-usage-original-vs-r14.md). Early documents may call components unknown or say later-successful versions untested; use their date and later evidence rather than treating every paragraph as current fact.

## 15. Working on another language/model or change

1. Identify exact model, board/version, updater format and hashes; record confidence and evidence separately.
2. Establish offline extraction and an unchanged roundtrip. Do not reuse the A910 DES key/marker without verifying it.
3. Read the device-side installer first: partitions, capacity, write order, update flags and recovery availability.
4. Determine CPU/OS/ABI, resources, fonts and language mapping for that target. Do not transfer addresses or hardware ioctls.
5. Start with one bounded resource/selector change. List allowed changed files; preserve original translations, metadata, kernel and installer where applicable.
6. Validate final artifacts independently, allocate complete installed size, retain margin and an exact rollback baseline. Record tests that were skipped.
7. Hardware testing requires a suitable functioning device and an understood recovery route. Offline results cannot authorize a claim of universal safe flashing.

For a contributor or AI: read README → this guide → BUILD → the specific source functions above. Search maintained code before re-running cryptographic search, reverse-engineering DWR or testing modern syscall ABI again. Extend other languages using their own gap reports and fonts. Record each new fact with input hash, command, output, model applicability and limitations. Never turn an unverified experiment into a release claim merely because it compiles.
