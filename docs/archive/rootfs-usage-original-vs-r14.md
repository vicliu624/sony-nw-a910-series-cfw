# Original 1.03 vs R14 system filesystem usage

**Result: R14 allocates 45,056 additional bytes (44 KiB, 11 blocks) in the same fresh ext2 model.**

Both roots were freshly extracted from SHA-256-pinned UPGs. The original Sony BusyBox mke2fs created separate ordinary 48 MiB images, using 4096-byte blocks and the same inode layout. debugfs allocated every file data block and required indirect blocks, directories and links. No player, real disk or mount was accessed.

| Metric | Original Sony 1.03 | R14 LOCAL1 | Difference |
| --- | ---: | ---: | ---: |
| Regular file contents (bytes) | 41,947,917 | 41,989,598 | +41,681 |
| Allocated model usage including ext2 metadata (bytes) | 45,019,136 | 45,064,192 | +45,056 |
| Unallocated model space (bytes) | 5,312,512 | 5,267,456 | -45,056 |

The additional allocation is approximately 0.08952% of a 48 MiB partition. Ext2 metadata and indirect-block changes contribute no further net blocks in this comparison.

| Modified/new file | Original bytes | R14 bytes | Content delta | Allocated data-block delta |
| --- | ---: | ---: | ---: | ---: |
| `./etc/init.d/gemini` | 3,290 | 3,424 | +134 | +0 |
| `./usr/local/bin/a918-boot` | 0 | 27,376 | +27,376 | +28,672 |
| `./usr/local/bin/a918-boot-dispatch` | 0 | 2,091 | +2,091 | +4,096 |
| `./usr/local/bin/gemini-ICX1101` | 5,691,632 | 5,691,632 | +0 | +0 |
| `./usr/local/share/gemini/res/dui_tex/ICX1101_home_anim.dpk` | 956,944 | 956,944 | +0 | +0 |
| `./usr/local/share/gemini/res/dui_tex/dui_tex.dpk` | 5,223,408 | 5,223,408 | +0 | +0 |
| `./usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk` | 248,000 | 260,080 | +12,080 | +12,288 |

New launcher: 28 KiB allocated. Dispatcher: 4 KiB allocated. Completed English wording: 12 KiB additional allocation. The slightly longer Sony startup script stays within the same block. The main program and both Chiikawa texture packs retain their original file lengths.

Kernel, initrd and the original update script are byte-identical to the original package. Games, ROM and save files do not appear in the system archive. Future app additions under APPS affect the visible data volume, not the rootfs image.

The encrypted UPG happens to be 51,392 bytes smaller than original because compressed resource bytes differ. This is NOT evidence of reduced installed filesystem usage.

Limits: 48 MiB is the original NAND driver default, not geometry read from the actual device. These numbers describe freshly allocated rootfs installation contents, not a live device df, runtime writes, NAND bad-block state or proof of flash/recovery safety. Original format-before-extract update behavior remains unchanged.

Evidence: `work/rootfs-usage/comparison.json`, `file-differences.json`, both capacity JSON reports and their allocation logs. Reproduce with:

```powershell
wsl -d Ubuntu-24.04 -- python3 scripts/check-rootfs-capacity.py work/rootfs-usage/original.tar.gz work/rootfs-usage/original-capacity.json
wsl -d Ubuntu-24.04 -- python3 scripts/check-rootfs-capacity.py work/rootfs-usage/R14.tar.gz work/rootfs-usage/R14-capacity.json
```
