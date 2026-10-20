# A918 Game completion audit — awaiting device evidence

Latest candidate: English 1.03 / Chiikawa R6 Game ALPHA2.
UPG SHA256: 7d23f2a8f24bc36f083b21477ba8c04cbb069e27c94f26f82d1cf3f9029f366b

Previous goal turn made concrete progress: replaced the FM radio drawing with
Ramen Armor, produced ALPHA2, independently decoded it, compared it with the
delivered ALPHA1 and preserved the old release. The unresolved device-validation
condition was present after ALPHA1, after ALPHA2 and again at this audit.

Current external-state check: Windows reports only its fixed C volume and an
unlettered fixed volume. Get-PnpDevice -PresentOnly found no friendly name
matching WALKMAN, NW-A91 or Sony. No device test results have been supplied.
There is no running build/test job to wait for. Further offline repetition
cannot prove the remaining hardware requirements.

| Requirement | Current authoritative evidence | Completion assessment |
| --- | --- | --- |
| English firmware artifact for A918 | ALPHA2 UPG and stock-updater ZIP exist; independent Rockbox decode and metadata checks pass | Artifact built; actual update/boot unverified |
| Preserve original nine functions | Original nine dispatch records, original resource files and fixed grid count unchanged; all non-allowlisted original main bytes restored/compared | Static preservation verified; device pages unverified |
| Game in Home page 2 | Native Home hook and companion second page in final ELF; actual ARM hook runs 167 instrumented cases | Implemented; real Sony event routing unverified |
| Native falling blocks using five-way controls | Shared C core, host sanitizers, ARM926 QEMU, WASM/browser tests | Logic verified locally; physical key mapping/display unverified |
| Pause, save and resume | Valid/invalid save tests and browser persistence; native atomic-write path present | Real /var writes, power-cycle persistence unverified |
| Stable return to the Sony player | Six actual ARM process scenarios, shared padded framebuffer restore and original watchman supervising the direct stopped/resumed test child | Recovery logic tested; real Gemini playback/UI recovery unverified |
| FM pending entry | Status page and Chiikawa icon present, driver/flags unchanged | Implemented; no claim of FM reception |
| Chiikawa Game and FM icons | Actual shared-core rendered previews inspected; ALPHA2 vs ALPHA1 differs only in built-in companion executable | Offline artwork integration verified |

The full goal is NOT complete. The last material gap is physical A918 evidence;
do not relabel local tests as proof of on-device stability, or generate another
revision without a concrete issue or user request.

## Minimum useful first device report

After manually installing the ALPHA2 package and following Sony's update
completion instructions, pause music for the first game test.

1. Does the player boot to its original Home with all nine items?
2. Does short OPTION on Home reach calibration/second page? If not, report
   exactly what appears; do not infer success from the updater finishing.
3. Does calibration recognize press/release for left, right, up, down, center,
   BACK and OPTION? Do Game/Anoko and FM/Ramen Armor render correctly?
4. Can Game start, move, rotate and pause/resume with the physical buttons?
5. BACK from play -> list -> second page -> original Sony Home. Repeat this
   round trip three times; verify the Sony controls and normal music playback.
6. Re-enter and Resume; then check persistence after a normal power cycle.
7. FM should show only UNVERIFIED/PENDING and return normally. Test USB changes
   during a game only after the ordinary return path has worked on the device.

These are observations to obtain, not tests that have already passed. Do not
rewrite the shipped verification JSON's hardware_verified field without evidence.
If entry fails before drawing, diagnostics are in /var/a918-games/handoff.log,
an internal Linux partition rather than the Windows WALKMAN music volume.
