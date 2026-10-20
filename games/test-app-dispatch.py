"""Mock services only. Does NOT mount or access a player."""
from pathlib import Path
import tempfile,subprocess,os,json,hashlib
ROOT=Path(__file__).resolve().parents[1];source=(ROOT/'games/boot-dispatch-apps.sh').read_text();cases=[]
for name in ['normal','no_apps','Walkman','pending_update','unknown_update','USB_connected','bad_RAM_mount','occupied_disk','mount_failed','stage_failed','app_crash','unmount_failed','USB_after_game']:
 with tempfile.TemporaryDirectory(prefix='a918-app-dispatch-') as tmp:
  r=Path(tmp);bin=r/'bin';bin.mkdir();disk=r/'disk';ram=r/'ram';trace=r/'trace';proc=r/'mounts'
  proc.write_text('none /tmp '+('ext2'if name=='bad_RAM_mount'else'ramfs')+' rw 0 0\n'+('/dev/mtdblock/2 /contents vfat rw 0 0\n'if name=='occupied_disk'else''))
  def script(path,s):path.write_text('#!/bin/sh\n'+s);path.chmod(0o755)
  script(bin/'nvpflag','echo '+('0x70555766'if name=='pending_update'else'garbage'if name=='unknown_update'else'0x00000000')+'\n')
  script(bin/'sony','echo sony >> "$TRACE"\n')
  script(bin/'mount',f'echo mount >> "$TRACE"\nexit {1 if name=="mount_failed"else 0}\n')
  script(bin/'umount',f'echo umount >> "$TRACE"\nexit {1 if name=="unmount_failed"else 0}\n')
  script(bin/'a918-boot',f'''echo "boot $*" >> "$TRACE"
case "$1" in
 --run-app) "{ram}/program" --games;exit $?;;
 --usb-present) if [ '{name}' = USB_connected ] || {{ [ '{name}' = USB_after_game ] && [ -f "$DONE" ]; }}; then exit 0; else exit 1; fi;;
 --choose) exit {0 if name=='Walkman'else 90};;
 --games) if [ -f "$DONE" ] || [ '{name}' = no_apps ]; then exit 0; else touch "$DONE";exit 100;fi;;
 --stage) [ '{name}' != stage_failed ] || exit 1
 echo '{r}' > '{ram}/cwd'
 printf '#!/bin/sh\necho app >> "$TRACE"\nexit {139 if name=='app_crash'else 0}\n' > '{ram}/program'
 chmod 700 '{ram}/program'
 exit 0;;
esac
''')
  s=source.replace('/usr/local/bin/a918-boot',str(bin/'a918-boot')).replace('/usr/local/bin/nvpflag',str(bin/'nvpflag')).replace('/etc/init.d/gemini',str(bin/'sony')).replace('/proc/mounts',str(proc)).replace('disk=/tmp/a918-game-disk','disk='+str(disk)).replace('ram=/tmp/a918-app-runtime','ram='+str(ram))
  begin=s.index('bounded() {');end=s.index('\ncleanup() {',begin);s=s[:begin]+'bounded() { "$@"; }\n'+s[end:]
  test=r/'dispatch';test.write_text(s)
  p=subprocess.run(['sh',str(test)],env={**os.environ,'PATH':str(bin)+':'+os.environ['PATH'],'TRACE':str(trace),'DONE':str(r/'done')},capture_output=True,timeout=10)
  lines=trace.read_text().splitlines()if trace.exists()else[]
  if name=='unmount_failed':assert p.returncode==91 and 'sony'not in lines and lines.count('umount')==3,(name,lines,p.stderr)
  else:assert p.returncode==0 and lines[-1]=='sony',(name,p.returncode,lines,p.stderr)
  if name in ['pending_update','unknown_update','USB_connected','Walkman','bad_RAM_mount','occupied_disk']:assert 'mount'not in lines,(name,lines)
  if name in ['normal','app_crash']:assert 'app'in lines and 'umount'in lines and lines.index('umount')<lines.index('sony'),lines
  if name=='stage_failed':assert 'app'not in lines and 'umount'in lines,lines
  assert not any('mtdblock/1'in x or 'mkfs'in x or 'fsck'in x for x in lines)
  cases.append({'name':name,'passed':True,'trace':lines})
helper=source[source.index('bounded() {'):source.index('\ncleanup() {')].replace('sleep 10','sleep 1').replace('sleep 2','sleep 1')
p=subprocess.run(['sh','-c',helper+'\nbounded sleep 20\n'],capture_output=True,timeout=4)
assert p.returncode!=0
cases.append({'name':'bounded helper terminates hung ordinary command','passed':True})
(ROOT/'work/game-apps/dispatch-tests.json').write_text(json.dumps({'dispatch_sha256':hashlib.sha256(source.encode()).hexdigest(),'cases':cases,'hardware_tested':False},indent=2));print(f'{len(cases)} dispatcher tests PASS')
