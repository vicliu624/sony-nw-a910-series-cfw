# A918 English R6 Game ALPHA2 — FM Chiikawa icon

User request: FM should also use a Chiikawa character rather than a radio drawing.

FM now uses Ramen no Yoroi-san (ラーメンの鎧さん): golden armor, white kitchen
headband and a ramen bowl. He is distinct from the silver Pochette Armor already
used by the original nine icons. Game retains Anoko. FM remains PENDING and has
no tuner/driver/audio activation. Character reference:
https://chiikawamarket.jp/collections/ramennoyoroisan

Artwork: built-in image_gen using A9.png only as a style reference. Selected
master assets/chiikawa/masters/A11-Ramen-Armor.png; full prompt and asset manifest
in assets/chiikawa/fm. Both new icons are 72x64 RGB565 plus alpha. They share the
same compositing code and selection highlight, on the existing R5 dark wallpaper.

Only icon asset compilation and second-page drawing changed. Falling blocks,
save format, controls, Sony hook and guardian/rescue logic are unchanged. The
independent verifier additionally compares every filesystem member to the
delivered ALPHA1 (SHA256 863fb1d923d4275307e73ae6aa7bc01a87d0fb86f99d0600fc01e6ced36722f1)
and requires that only /usr/local/bin/a918-games differs. All previous ALPHA1
firmware/ZIP/report files are retained. New files are named ALPHA2 separately.

The architecture and first device-test procedure remain as described in
game-r6-alpha1.md, bundled as technical-notes-base-alpha1.md. The current build,
verify and package scripts use scripts/game_release.py as their release label.

Hardware remains unverified: A918 boot, actual Home key route, calibration,
display ownership, playback after return, repeated round trips and USB/power
behavior still require the device. This revision does not change that status.
