# R2 concise wording completion

The R1 language selector was reported to work on the user's device, but many labels were blank. Parsing all DWR records confirmed blank original resources, not a shortage of numbered records: Japanese has 648 nonempty records of 826; English 360 and Simplified Chinese 359.

R2 translates all 288 English and 289 Chinese gaps. Existing target-language records are preserved. Japanese-empty records remain untouched. Wording records retain dynamic insertion line indicators from Japanese, including the empty lines used for track/channel/video names and the free-space number. Original target metadata is retained. New text is appended to each target DWR's UTF-16 and line-length pools; offsets and DPK section positions are rebuilt. Other language sections are byte-identical.

Translations use short menu labels and compact prompts. Newly translated lines are limited to 20 display-width units (ASCII 1; fullwidth/CJK 2), and line counts cannot exceed the Japanese source. This is a conservative text-length check, not a pixel-level rendering proof. Examples: Record / 录音; Bit Rate / 比特率; Channel List / 频道列表; Battery Low / 电量不足. Warnings preserve numeric limits and required actions.

Both final UPGs are independently extracted by Rockbox, then the resulting rootfs wording is parsed and compared against original Japanese and original target-language records. Expected result: 648 nonempty records, zero missing vs Japanese. The tar contains changes only to the main application selector instruction and wording DPK. All other member contents, names, permissions, ownership, times, links and special-file metadata are checked against the original archive. UPG components 0 through 2 remain original.

Build source: translations/missing.tsv. Outputs: firmware/experimental/*R2_COMPLETE_EXPERIMENTAL.UPG. Build and final verification reports reside in firmware/experimental/build-report-r2.json and work/completed-wording/final-verification.json.

R2 has not been installed. Chinese glyph coverage, pixel layout and full on-device behavior remain unverified. The version remains 1.03; the original updater may reject reinstalling the same version, so the same-version update path must be checked separately. No version spoof or device write was performed in this build step.
