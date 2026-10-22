#!/bin/sh
# R14 LOCAL: RAM launcher + disk app packages; no NVP/raw/root writes.
disk=/tmp/a918-game-disk
ram=/tmp/a918-app-runtime
owned=0
bounded() {
 "$@" & task=$!
 ( sleep 10; kill -TERM "$task" 2>/dev/null; sleep 2; kill -KILL "$task" 2>/dev/null ) & timer=$!
 wait "$task"; result=$?
 kill "$timer" 2>/dev/null
 wait "$timer" 2>/dev/null
 return "$result"
}
cleanup() {
 if [ "$owned" = 1 ]; then
  attempt=0
  while ! bounded umount "$disk"; do
   attempt=$((attempt+1))
   [ "$attempt" -lt 3 ] || return 1
  done
  owned=0
 fi
 rm -f "$ram/program" "$ram/program.tmp" "$ram/cwd"
 return 0
}
sony() {
 cleanup || { echo 'A918: cannot release game disk; Sony withheld' >&2; exit 91; }
 trap - HUP INT TERM
 A918_BOOT_BYPASS=1
 export A918_BOOT_BYPASS
 exec /etc/init.d/gemini
}
trap 'sony' HUP INT TERM
fup=$(/usr/local/bin/nvpflag -x fup) || sony
# Sony's update request is the exact fWUp word, not every nonzero word.
# A zero-only test also rejects the erased 0xFFFFFFFF value. Validate the
# complete read, defer pending updates to Sony, and never write the NVP flag.
case "$fup" in
 0x70555766) sony;;
 0x????????) case "${fup#0x}" in *[!0-9a-fA-F]*) sony;; esac;;
 *) sony;;
esac
/usr/local/bin/a918-boot --usb-present
[ "$?" = 1 ] || sony
/usr/local/bin/a918-boot --choose
[ "$?" = 90 ] || sony
/usr/local/bin/a918-boot --usb-present
[ "$?" = 1 ] || sony
busy=0
ram_ok=0
[ -r /proc/mounts ] || sony
while read source target filesystem options rest; do
 case "$source" in /dev/mtdblock/2|/dev/mtdblock2) busy=1;; esac
 [ "$target" != "$disk" ] || busy=1
 if [ "$target" = /tmp ]; then
  case "$filesystem" in ramfs|tmpfs) ram_ok=1;; esac
 fi
done < /proc/mounts
[ "$busy" = 0 ] && [ "$ram_ok" = 1 ] || sony
mkdir -p "$disk" "$ram" || sony
chmod 700 "$ram" || sony
bounded mount -t vfat -o rw,nosuid,nodev,noexec /dev/mtdblock/2 "$disk" || sony
owned=1
while :; do
 /usr/local/bin/a918-boot --usb-present
 [ "$?" = 1 ] || break
 /usr/local/bin/a918-boot --games
 selected=$?
 [ "$selected" -ge 100 ] && [ "$selected" -le 131 ] || break
 /usr/local/bin/a918-boot --usb-present
 [ "$?" = 1 ] || break
 index=$((selected-100))
 if bounded /usr/local/bin/a918-boot --stage "$index"; then
  cwd=$(cat "$ram/cwd")
  ( cd "$cwd" && /usr/local/bin/a918-boot --run-app )
 fi
 rm -f "$ram/program" "$ram/program.tmp" "$ram/cwd"
done
sony
