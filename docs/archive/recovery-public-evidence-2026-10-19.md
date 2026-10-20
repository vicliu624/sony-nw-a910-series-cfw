# Public A910 recovery evidence audit — 2026-10-19

User constraint: stop speculative board probing, UART wiring, shorting or further disassembly. Only propose a physical recovery procedure when supported by an identified same-board service document or a verifiable same-family, matching-failure successful recovery account. No hardware procedure was performed during this search.

## Target failure

NW-A918 failed an R12 update and repeats the failed-update path after reset. User has already established no usable USB enumeration window. Screens and buttons are now removed; battery and USB disconnected. Do not request repeat reset/USB-window tests or infer pin functions from unlabeled photos.

## Result

No publicly verifiable successful NW-A916/A918/A919 recovery tutorial matching failed update plus absent USB enumeration was found in this bounded English/Japanese/Chinese search. This is a search result, not proof that no repair procedure exists. No confirmed UART/boot-strap/test-point map was found.

## Sources and exclusions

1. Sony detailed operating guide, page 185, old PDF URL https://www.sony.jp/ServiceArea/impdf/pdf/3283404041.pdf . Search-index text explicitly contains the firmware-update-failed message and instructs retrying the update according to the computer screen. It does not provide a verified way to restore USB enumeration. The old URL currently redirects to manual support and some current support requests timed out; do not represent the full current service documentation as inspected.
2. Sony normal troubleshooting: https://support.sony.jp/electronics/support/digital-music-players-other-digital-music-players/nwd-w263/articles/S1110278026242 . Covers reset/charging/support, not evidence of an A918 forced-download entry. User has already exhausted the relevant reset observation.
3. A910 battery/connector repair examples: https://postrepair.net/repair-results/nw-a910_chargeconnecter/ and https://postrepair.net/repair-results/walkman-nw-a916_200426/ . Actual hardware repair/disassembly, but no demonstrated firmware-loop recovery or bootstrap pad map. Do not call these unbrick guides.
4. https://github.com/manuaguirre9/nw-a820-homebrew . README states experimental, tested on one Japanese NW-A829, and installation requires the player connected in Sony normal USB mode. This is an A820 homebrew project, not evidence that an unenumerated A918 can be recovered. Do not run its installer on this device or claim hardware compatibility.
5. Same-family user reports about database errors and failed playback were found, but they do not establish successful repair of this specific update/USB failure. Generic service-manual listing pages without an inspected, matching document were not accepted as evidence.

## Boundary

No further guessed test-point identification, NAND/USB shorting, UART voltage assumptions, blind NVP edits, other-model firmware or other-model boot-key sequences. Existing stock-code emergency paths remain research findings, not a user-executable recovery method. Finding code that can expose USB does not establish an entry path in the present update stage.
