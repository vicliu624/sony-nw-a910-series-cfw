# R7 Directional ALPHA3 — instant Tetris

User supersedes the earlier pause/persistence requirement: make it a simple
instant game. The app removes pause, save/resume UI and app persistence APIs.
Game list contains Tetris and Back to Home only. Enter starts a new round;
center immediately ends it and returns to the list. Directions remain
left/right movement, up rotate, down soft drop.

The cooperative integration removes all file/directory operations. Its only
Linux syscall is gettimeofday (78). Old saves are neither read nor altered.
The generic engine's isolated serialization tests are not a game feature;
they have no app API or native input path.

OPTION/BACK retain stock-event passthrough: the overlay restores Sony buffers
and returns the original event, without saving or pausing. Directional edge
paging and the unverified FM status page remain.

Tests: 100 fresh rounds in C, 20 in WASM, browser re-entry/reload without
storage, compiled ARM entry/play/center-exit/new-round/FM/return, 20 OPTION
phase/context cases. Replay rejects all syscalls except 78 and asserts that
pause/save app APIs are absent. Physical A918 behavior still requires testing.
