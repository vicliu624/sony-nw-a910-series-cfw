Sony NW-A910 series custom firmware research: Chinese/English localization and examples of device customization. Only NW-A918 has device-test reports.

This release provides separate Simplified Chinese (zh-CN) and English (en) firmware packages based on Sony 1.03. Each is an R14-derived combined demonstration; neither is an on-device language-switch or localization-only build.

- Completed Simplified Chinese and English wording in separate firmware variants.
- Three-language technical guides document all 11 embedded language resources, reverse engineering, resources, ABI, capacity gates and model-porting limits.
- Chiikawa artwork demonstrates resource replacement; it is an example, not a required project identity.
- WALKMAN / APPS startup selection; original Sony Home and OPTION behavior retained.
- Experimental custom-code examples: dynamic program discovery, Tetris and color iD. These demonstrate extensibility, not a general app-platform commitment.
- Apps, ROM and saves stay on user storage. Commercial ROMs and initialization snapshots are not included.
- Package contains the original Sony Windows updater, generated UPG, app sources, checksums and offline verification.

Alpha.3 fixes the startup update-flag check and Windows CRLF app manifests. The user confirmed the local English alpha.3 app list and installed applications work on NW-A918 on 2026-10-22. CI builds the same source with offline checks; cross-toolchain byte identity is not claimed. The Chinese build and full hardware qualification remain outstanding.

Intermittent external-storage I/O errors remain under investigation; successful app launch does not establish transfer or save durability.

Not fully hardware-qualified. A previous version failed installation, and the affected device has not been confirmed recovered. Original format-before-extract installation remains; this is not a recovery package. ROM-based tests are excluded from public CI. Review docs/SAFETY.md before any installation.
