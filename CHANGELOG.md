# Changelog

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
