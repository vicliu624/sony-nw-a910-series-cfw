# Chiikawa B1 resource preparation

User approved nine distinct characters and B1 green meadow reference. Source URLs recorded in work/chiikawa-review/candidates.json. The nine official reference portraits contain names; ImageGen extracted character cutouts and adapted the approved B1 image into a quiet portrait background. Edited master images are copied into assets/chiikawa/masters; original source downloads remain in work/chiikawa-theme/sources.

The conversion script mechanically crops transparent margins, resizes cutouts, composites grass under icon tiles, and encodes RGB565 DTX. It builds 81 numbered animation frames and 14 matching static/home-hot textures. Every modified DTX retains its original header and byte size. Nested DPK sizes, headers and offsets are retained. No firmware is built or installed at this stage.

Output directory: outputs/A918-Chiikawa-B1 in the chat workspace. It contains characters, icons, DTX resources, patched DPK resource packs, a 240x320 wallpaper and HTML design preview. The menu preview places the icon grid at an assumed y=40; actual application layout and animation sequence have not been verified on hardware. A schematic state selection interface is included for inspection.

Important distinction: the resource-only pack composites background into the nine icon tiles. The existing application can still paint its original black outside those tiles. The full-screen meadow preview additionally changes background and text-panel presentation and is only a design target, not an implemented firmware feature. Four extracted full-screen DTXs belong to other screens (ring, refresh, power-off, Walkman logo); none was verified as the home background. Full-screen background integration requires further home-page rendering analysis before a complete firmware can be claimed.

## English R3 firmware built

The user subsequently requested an English firmware containing these resources. `scripts/build-chiikawa-firmware.py` builds it from the complete English R2 baseline and the two compiled resource packs copied into `assets/chiikawa/compiled/resource-packs`. Only these two DPK files change relative to R2. All 613 archive entries and their metadata were compared; all non-texture file contents are identical to R2. The result was independently extracted using Rockbox and checked with `scripts/verify-chiikawa-firmware.py`. No device write occurred. The background limitation above remains applicable.

Artifact: `firmware/experimental/NW-A918_1.03_English_Chiikawa_B1_R3_EXPERIMENTAL.UPG`.
SHA256: `cbcdfbed2b91db968956c9a30e5a7e4e22816e9c09c69df6272644023d41694a`.
Installer directory: `work/install-english-chiikawa-b1-r3`; original Sony updater with candidate named `NW_A91X.UPG`. Version remains 1.03 and same-version reinstall is not bypassed.

## Dark R4 after hardware feedback

User photo confirms R3 installed and displayed all nine characters. Original header, title and playback bars are separate drawing regions. The photo also shows all nine fixed pink frames, so the initial assumption that numbered/hot resources would only appear for one selected item was wrong. R4 removes fixed frames from every icon texture and keeps size animation, whose visual selection behavior still needs testing. Its navy wallpaper master is assets/chiikawa/masters/Dark.png. The title/footer rendering is unchanged; navy texture background reduces contrast at boundaries without claiming full-screen coverage.

Build scripts accept `--dark`. Compiled resources are assets/chiikawa/compiled-dark-r4/resource-packs. Output artifact is NW-A918_1.03_English_Chiikawa_Dark_R4_EXPERIMENTAL.UPG, SHA256 c6563968e4cefa1e75c5a7fe178e668d8123b7986b9b3191a110e570de059bbf. Independent Rockbox extraction and all 613 rootfs member comparisons passed. No R4 flashing was performed.

## R5 footer resource discovery and replacement

Further binary/resource analysis found the home label background as a 240x44 format-5 DTX under ICX1101_home_anim.dpk, leaf key 0xe60ccd8f, parent key 0xaa48c2d0. Its bytes decode to the original blue gradient visible in the user photo. The application loads this key into texture table slot 21 at VA 0x8c3ec..0x8c3f8 (literal VA 0x8d2e0). The home constructor fetches slot 21 at VA 0x287a4..0x287b0; its footer region is configured at y=244, height=44, width=240 at VA 0x28830..0x28864. Grid construction at VA 0x28540..0x285cc establishes 80x72 tiles starting at y=28. The separate 240x32 indicator_bg.dtx matches the native bottom playback strip and is shared with other pages.

R5 (`--footer`) slices one wallpaper at y28..244, 244..288, 288..320 into grid, home footer and status assets. A common smooth darkening below y220 keeps native white text legible. Fixed icon frames remain removed. Foreground text/indicators and top header remain original. The shared status-strip change affects other pages too.

The user initially requested removing the lower trio, then accepted the shown dimmed-trio design; final R5 retains that accepted version. A separate clean-background image edit was requested but is not used in this artifact.

Artifact SHA256: 1055a6689c762ad12de0e629461b42823bb812584920b9341f29ca75c4160ec5. Independent Rockbox extraction and all-member comparisons passed. No R5 device write was performed. See verification-footer-r5.json for exact crop and white-contrast checks.
