# Checked resource deltas

These zlib-compressed XOR deltas preserve the existing R14 English/Chiikawa baseline without storing full original Sony system files. manifest.json binds the original UPG, each input file, delta and resulting file with SHA-256. They cover the stock language instruction, completed wording, two theme packs and init wrapper. The runtime launcher/dispatcher are compiled/copied from source separately.

These are maintained binary patch inputs, not a claim that theme artwork was regenerated from editable source in CI. Original Sony and character rights still apply; see THIRD_PARTY.md. Historical editing sources are retained locally under archive/.

Selected-language wording and the language instruction are regenerated after these baseline deltas. Edit translations/missing.tsv / scripts/wording.py for translations; see docs/TECHNICAL.md in the project root for the full procedure. Chiikawa is an illustration of resource replacement, not a required theme for the method.
