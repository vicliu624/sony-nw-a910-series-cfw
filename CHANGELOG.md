# Changelog

## 0.1.0-alpha.3 (2026-10-22)

- Fix the boot dispatcher's zero-only update-flag check. Validate a complete 32-bit hexadecimal read and defer the exact Sony `fWUp` request; an erased/non-request value no longer suppresses the chooser. No NVP writes or installer changes.
- Accept Windows CRLF as well as LF in external application manifests, keeping the existing path, ABI, size, CRC and ELF checks.
- Exercise the built ARM chooser and Tetris workers, including five-way keys, partial events, USB exit, failure paths and resource release. Package the resulting report.
- Replay the original Sony kernel's ELF admission instructions against all three built programs and invalid-header controls; retain the boundary that this is not a full hardware exec test.
- Test RAM staging and the independent watcher with actual ARM applications through an explicit host QEMU adapter. Add local-only real-ROM color-app UI tests; CI still excludes ROMs and does not claim hardware validation.
- Document REA/Ghidra findings and the remaining validation limits in docs/APP_STARTUP.md. On 2026-10-22 the user confirmed that the local English alpha.3 firmware shows the app list and runs the installed apps as expected. This is a user device report, not full hardware qualification; the Chinese build remains untested on hardware. Earlier packages remain unchanged.

## 0.1.0-alpha.2

- Build separate Simplified Chinese and English firmware packages in a CI matrix; regenerate selected wording from TSV and verify language instruction.
- Promote resource inspection, DWR parsing/completion and reviewed delta update helpers into maintained source.

- Restore localization and reusable Sony NW firmware research as the project focus; theme/game changes are demonstrations. Add complete English, Chinese and Japanese technical handoffs.

- Rename the project to sony-nw-a910-series-cfw and clarify that only NW-A918 has device-test reports. Historical packages and firmware behavior are unchanged.

## 0.1.0-alpha.1

First SemVer package line, based on the R14 external-app implementation. This does not change Sony's 1.03 version or establish hardware compatibility.

- Add a portable original-firmware-to-installation-package build, independent of R13 and the sibling desktop project.
- Add GitHub Actions validation, artifact uploads and draft prereleases.
- Add English, Simplified Chinese and Japanese READMEs and current build/app/safety documentation.
- Archive superseded scripts and investigations; exclude generated and private files from Git.
- Public CI packages omit commercial game ROMs and seed saves. Application behavior is unchanged.
