# Original 1.03 pruning assessment

This is an offline assessment, not a new firmware release. No existing UPG or release ZIP was changed. No device was accessed and no hardware program was executed. Original Sony payload SHA-256: cfbcc372e2a18f11c12a080c9e5f039333e09175f5db03ac841a4227d3896b32.

## Recommended first investigation: non-loaded debug information

16 user-space ELF files retain debug information. On ordinary copies, arm-linux-gnueabi-strip --strip-debug saves 123,081 file bytes. Every program header, entry point, ELF flags and every SHF_ALLOC section address, size, type and byte content are identical before/after. No kernel module was stripped. The main player is already stripped; the savings come from small utilities such as mkdosfs, flash utilities, USB/diagnostic tools and FM test utilities.

A full fresh ext2 allocation simulation reclaims 122,880 bytes (120 KiB / 30 blocks): 116 KiB data blocks plus one 4 KiB indirect block. Free model space rises from 5,312,512 to 5,435,392 bytes. This exceeds R14's 44 KiB increase without deleting a program or media resource. It remains a low-relative-risk candidate, not hardware execution proof. Debug information should remain archived on the computer. Changes to rescue-related executable containers still need testing.

## Large candidate: factory audio input files

/usr/local/share/mptapp/oma contains seven Test008.oma through Test014.oma files, totalling 5,787,846 bytes (about 5.52 MiB). Their filenames and directory are directly referenced by mptapp. No exact references to these sample paths/names were found in the normal player. That supports, but does not prove, separation from normal playback: runtime path construction is possible.

Removing ONLY those seven files from an ordinary archive copy and rerunning full allocation frees 5,824,512 bytes (about 5.55 MiB). Free model space rises from 5.07 to 10.62 MiB. This would remove factory audio test inputs. Do not claim normal playback, factory startup, calibration or recovery safety from capacity tests alone. Relocating samples to visible storage and loading them on demand is preferable if factory tests must remain available, but requires explicit path/fallback changes and testing. An unconditional data-volume symlink is insufficient when the volume is unavailable or owned by USB.

## Other candidates, not automatically safe to remove

| Group | File content size | Assessment |
| --- | ---: | --- |
| Other factory RGB/YUV/channel assets | 2,208,104 bytes (~2.11 MiB) | Used by mptapp display/video/TV tests; may be needed at factory UI startup. |
| Factory executable mptapp | 2,798,392 bytes (~2.67 MiB) | Explicit original startup branch for factory test flags. Keep while repair access is unresolved. |
| All factory assets, including OMA above | 7,995,950 bytes (~7.63 MiB) | This overlaps the first two asset rows; do not add twice. |
| Non-English/non-Japanese loader BMPs | 1,593,888 bytes (~1.52 MiB) | 36 images; 1,695,744 rounded data bytes. These include update/error/low-battery/HOLD screens. Locale selection is separate from the patched player UI; retain emergency/update resources for now. |
| Entire /usr/local/test directory | 355,182 bytes (~347 KiB) | Not removable as a group: normal boot invokes watchdog_test. FM and key tests are also useful for research. |
| Two OMF debug libraries | 46,220 bytes | Low saving, unresolved dynamic library selection. No recommendation to remove. |

The factory executable plus all factory assets contain about 10.29 MiB, but deleting that whole subsystem would sacrifice original service/test capabilities. It is not a safe default recommendation.

## Preserve

- /usr/local/test/watchdog_test: etc/rc2.d/S90icx1087helper invokes the helper script, which actively starts this binary.
- mptapp and repair/USB helpers: the original startup script has factory branches and separate emergency branches using apl_conn, mgscsi_test_ioctl and load_usb_epson. Do not equate normal-boot inactivity with dispensability.
- Both fonts: total 7,250,896 bytes (~6.92 MiB). Generic/model-specific resources are not proven duplicates. An English interface still needs Japanese/Chinese glyphs for song metadata.
- The main player, audio DSP and codecs, USB/storage drivers, init, BusyBox and required C/C++/OMF libraries. ELF dependencies show shared use by normal playback and factory tests. Do not strip kernel modules with user-space ELF rules.
- Existing television/recording functionality: removing its codecs or DSP resources changes original features, contrary to the present goal of keeping the Sony player intact.

Many shell commands are symlinks into one BusyBox executable. Removing command names does not reclaim the whole BusyBox binary; rebuilding a smaller BusyBox would require the exact old-platform configuration and validation.

## Recommendation

Start with debug-only stripping and preserve all functions and assets. If a larger margin is later needed, evaluate externalizing factory OMA samples, with a clear unavailable-media fallback. Leave factory executables, watchdog, fonts, playback codecs and rescue/update resources intact until there is a verified recovery path and hardware testing. R14's apps already live outside rootfs, so adding apps does not justify removing megabytes of original player functionality.

Evidence: work/stock-trim-audit/assessment.json, strip-results.json, both candidate capacity reports and their allocation logs. All simulations use ordinary files and the original Sony mke2fs on the driver-default 48 MiB model. These are not live df readings or flash safety guarantees.
