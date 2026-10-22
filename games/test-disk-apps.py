"""ARM/QEMU tests on ordinary fixture directories only; no devices/mounts."""
from pathlib import Path
import subprocess,shutil,json,hashlib,os
ROOT=Path(__file__).resolve().parents[1];work=ROOT/'work/game-apps';fixture=work/'fixture';ram=work/'ram';cases=[]
if fixture.exists():shutil.rmtree(fixture)
fixture.mkdir();ram.mkdir(exist_ok=True)
app_source=Path(os.environ.get('A918_APP_FIXTURES',str(ROOT/'work/ci/apps')))
for name in ['tetris','tamagotchi-id']:
 shutil.copytree(app_source/name,fixture/name)
 if (fixture/name/'app').read_bytes()!=(work/name).read_bytes():
  raise RuntimeError('App fixture does not match this build: '+name)
flags=['-std=c99','-Os','-march=armv5te','-marm','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-ffunction-sections','-fdata-sections','-DA918_EXTERNAL','-DA918_LAUNCHER',f'-DAPP_ROOT="{fixture}"',f'-DAPP_RAM="{ram}"','-nostdlib','-static','-Wl,--gc-sections','-Wl,-e,_start','-Wl,--build-id=none']
subprocess.run(['arm-linux-gnueabi-gcc',*flags,'games/src/app_launcher.c','games/src/runtime.c','games/src/boot_arm.c','games/src/sony_oabi.S','-lgcc','-o',str(work/'launcher-test')],cwd=ROOT,check=True)
def run(*args,cwd=ROOT,binary='launcher-test',expected=0):
 p=subprocess.run(['qemu-arm','-cpu','arm926',str(work/binary),*args],cwd=cwd,capture_output=True,timeout=30)
 if p.returncode!=expected:raise RuntimeError((args,p.returncode,p.stdout,p.stderr))
 return p.stdout.decode()
def check(name,fn):fn();cases.append({'name':name,'passed':True})
def eq(a,b):
 if a!=b:raise RuntimeError((a,b))
check('getdents discovers and sorts both disk apps',lambda:eq(run('--catalog-test'),'tamagotchi-id\ntetris\n'))
manifest_path=fixture/'tetris/app.ini';manifest_lf=manifest_path.read_text().replace('\r\n','\n')
manifest_path.write_bytes(manifest_lf.replace('\n','\r\n').encode('ascii'))
check('Windows CRLF app manifest remains discoverable',lambda:eq(run('--catalog-test'),'tamagotchi-id\ntetris\n'))
manifest_path.write_bytes(manifest_lf.encode('ascii'))
check('Tetris stages byte-identically in RAM',lambda:(run('--stage','1'),eq((ram/'program').read_bytes(),(fixture/'tetris/app').read_bytes())))
check('Tetris standalone 200 rounds and center exit',lambda:run('--app-test',binary='tetris'))
# QEMU does not install a host binfmt handler for ARM execve. The adapter below
# is used only in this ordinary fixture directory, never in packaged firmware.
# Exercise the real RAM staging + process-group watcher + actual ARM app chain.
def staged_app(app_id,index):
 run('--stage',str(index))
 staged=ram/'program.arm';(ram/'program').replace(staged)
 eq(staged.read_bytes(),(fixture/app_id/'app').read_bytes())
 interpreter=shutil.which('qemu-arm')
 if not interpreter:raise RuntimeError('qemu-arm is required')
 (ram/'program').write_text('#!/bin/sh\nexec "'+interpreter+'" -cpu arm926 "'+str(staged)+'" --app-test\n')
 (ram/'program').chmod(0o700)
 run('--run-app',cwd=fixture/app_id)
 staged.unlink()
check('staged Tetris executes ARM code under independent watcher and returns',lambda:staged_app('tetris',1))
if '--without-rom' not in __import__('sys').argv:
 check('color standalone real ROM / save / new pet / restore',lambda:run('--app-test',binary='tamagotchi-id',cwd=fixture/'tamagotchi-id'))
 check('color disk save exists',lambda:eq((fixture/'tamagotchi-id/state.sav').is_file(),True))
 check('staged color executes real ROM under independent watcher and returns',lambda:staged_app('tamagotchi-id',0))
original=(fixture/'tetris/app').read_bytes();bad=bytearray(original);bad[-1]^=1;(fixture/'tetris/app').write_bytes(bad)
check('damaged executable rejected before execution',lambda:run('--stage','1',expected=1))
check('failed stage removes executable',lambda:eq((ram/'program').exists(),False))
(fixture/'tetris/app').write_bytes(original)
manifest=(fixture/'tetris/app.ini').read_text();(fixture/'tetris/app.ini').write_text(manifest.replace('exec=app','exec=../other'))
check('manifest path traversal excluded',lambda:eq(run('--catalog-test'),'tamagotchi-id\n'))
(fixture/'tetris/app.ini').write_text(manifest+'id=tetris\n')
check('duplicate manifest fields excluded',lambda:eq(run('--catalog-test'),'tamagotchi-id\n'))
(fixture/'tetris/app.ini').write_text(manifest.replace('api=1','api=99'))
check('unknown application ABI excluded',lambda:eq(run('--catalog-test'),'tamagotchi-id\n'))
(fixture/'tetris/app.ini').write_text(manifest)
(fixture/'tetris/app.ini').unlink();(fixture/'tetris/app.ini').symlink_to(fixture/'tamagotchi-id/app.ini')
check('manifest symlink rejected',lambda:eq(run('--catalog-test'),'tamagotchi-id\n'))
(fixture/'tetris/app.ini').unlink();(fixture/'tetris/app.ini').write_text(manifest)
check('out of range index rejected',lambda:run('--stage','99',expected=1))
check('non-numeric index rejected',lambda:run('--stage','../1',expected=1))
for name in ['tetris','tamagotchi-id']:shutil.rmtree(fixture/name)
check('empty APPS directory supported',lambda:eq(run('--catalog-test'),''))
# Independent watcher uses harmless host shell fixtures, never a mounted player.
(ram/'program').unlink(missing_ok=True)
check('missing staged executable returns explicit exec failure',lambda:run('--run-app',expected=127))
(ram/'program').write_text('#!/bin/sh\nexit 0\n');(ram/'program').chmod(0o700)
check('independent launcher watcher reaps normal app exit',lambda:run('--run-app'))
(ram/'program').write_text('#!/bin/sh\nkill -KILL $$\n')
check('independent launcher watcher reaps crashed app',lambda:run('--run-app',expected=85))
(ram/'program').write_text('#!/bin/sh\neval "printf \\\\001 >&$A918_APP_HEARTBEAT_FD"\nsleep 7\n')
check('independent watcher kills stalled app after heartbeat',lambda:run('--run-app',expected=84))
for app in ['tetris','tamagotchi-id']:
 read,write=os.pipe()
 p=subprocess.run(['qemu-arm','-cpu','arm926',str(work/app),'--app-heartbeat-test'],env={**os.environ,'A918_APP_HEARTBEAT_FD':str(write)},pass_fds=(write,),timeout=5)
 os.close(write);data=os.read(read,16);os.close(read)
 if p.returncode or data!=bytes([1,2,1]):raise RuntimeError((app,p.returncode,data))
 cases.append({'name':app+' ABI heartbeat/save-grace integration','passed':True})
report={'scope':'QEMU ARM926 ordinary fixture files; no hardware','rom_tests_run':'--without-rom' not in __import__('sys').argv,'cases':cases,'programs':{n:hashlib.sha256((work/n).read_bytes()).hexdigest()for n in ['a918-boot','tetris','tamagotchi-id']}}
(work/'app-tests.json').write_text(json.dumps(report,indent=2));print(f'{len(cases)} disk application tests PASS')
