# Findings, 2026-10-17

## Firmware acquisition

The live Sony download landing page is unavailable. The Wayback snapshot of the official page exposes the original download link:

- Historical page: https://web.archive.org/web/20110915234423id_/http://www.sony.jp/support/walkman/download/nw-a916_fw.html
- Live original file: https://walkman.update.sony.net/fw/pc/A910/NW-A910_V1_03.exe
- Original updater size: 13,628,920 bytes; historical page describes 13,310 KB.
- SHA256: `6f745a8ff3c90b7abcd5ea185e6f51e4598492f6de5bdb3256599c6f43e39701`.
- Authenticode checked locally: Valid, Sony Corporation. This concerns the Windows wrapper; it does not establish the UPG's internal signature scheme.

## Extracted updater

7-Zip detects a 32-bit PE wrapper with an embedded MSZip CAB. Extracted without launching the updater. Main files: FWUpdater.exe, FWUpdaterCom.dll, FWUpdaterRes.dll, NW-A91X.ini, NW_A91X.UPG and multilingual PC license files. Multilingual PC licenses are not evidence that the player's firmware contains multilingual menus.

NW-A91X.ini explicitly includes NW-A918 and points to NW_A91X.UPG. The field `Signature=dbmn` is an updater configuration value; it is not evidence of a cryptographic firmware signature.

## Tools

REA: https://github.com/morluto/rea . Its documented firmware workflow delegates extraction to Binwalk/Unblob on Linux; native analysis uses an external analysis engine. Useful for later structured inspection, not a Sony-specific decoder.

Rockbox upgtools source downloaded into tools/rockbox-upgtools from https://github.com/Rockbox/rockbox/tree/master/utils/nwztools/upgtools . Its upg.c known model/KAS table has no NW-A910 entry. It supports explicit key/signature inputs and key-search methods. Compatibility with this payload remains unverified; do not substitute another model's key without validating container integrity.

## Remaining work

1. Identify UPG encryption and integrity structure; inspect updater DLL if needed.
2. Extract and classify components.
3. Locate actual on-device menu resources and language selection logic.
4. Establish lossless round-trip repacking before any translation changes.
5. Determine recovery options before any device write. No device writes performed.
