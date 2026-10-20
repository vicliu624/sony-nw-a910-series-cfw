# Experimental custom-code example

This is a demonstration of device customization, not the main project purpose or a general app-platform promise. Localization and the reusable firmware method are documented in [the technical guide](TECHNICAL.md).

The launcher scans `APPS/<id>/app.ini` on the visible WALKMAN data volume. At most 32 valid app directories are listed, sorted by ID, with five rows per page. Select RESCAN after changing files. Applications are independent of the firmware package.

```ini
api=1
id=tetris
name=TETRIS
version=1.0.0
exec=app
bytes=12345
crc32=12345678
```

IDs contain 1–32 lowercase ASCII letters, digits or hyphens. Display names contain at most 32 uppercase letters, digits, spaces or hyphens. The program length and CRC32 must match the actual `app`. The launcher accepts static ARMv5 soft-float ELF programs using Sony's OABI syscall convention, at most 2MiB. CRC is damage detection, not trust or authentication.

The executable is copied to verified ramfs/tmpfs before execution. The app directory is its working directory. Apps receive `--games` and `A918_APP_HEARTBEAT_FD`; send byte 1 at least every five seconds, or byte 2 while actively saving (30-second grace). Startup grace is 30 seconds. A supervised process group is terminated on crash/stall, subject to kernel I/O limitations. Trusted apps run with system privileges; there is no sandbox.

Tetris uses directions to move/rotate/drop; center ends the round and returns. No save or pause. iD uses left=A, center=B, right=C; down opens Continue / Save and Exit / New Game / Restore Backup. It is silent and renders at 128×128.

The ROM-free iD app requires `APPS/tamagotchi-id/rom.bin` (4MiB, currently CRC32 `0c2e4116`) and the compatible `new.sav` initial snapshot. ROM and seed files are not provided by CI. Without them the game is not a ready-to-play download. An existing compatible R14 app directory can supply these private inputs; back it up before replacing files. New-game/restore requires compatible state metadata, not just a correctly sized ROM. Saves are `state.sav`, `state.bak`, and temporary `state.tmp` in the same directory. Do not replace saves during an app update.

R13 used `GAMES/TAMAGOTCHI/ID/`; migration is manual. Back up state.sav/state.bak, then copy to the R14 app directory. No automatic migration is claimed.
