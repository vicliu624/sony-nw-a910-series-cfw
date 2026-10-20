"""WSL-only offline hard gate. Formats ONLY an explicitly bounded regular image.
Uses Sony's original BusyBox mke2fs and fully allocates every tar file block.
Never mounts anything and never opens a block device. Requires >=4MiB headroom.
"""
from pathlib import Path,PurePosixPath
import argparse,tarfile,subprocess,re,hashlib,json,os
ROOT=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser();ap.add_argument('archive');ap.add_argument('report');args=ap.parse_args()
archive=(ROOT/args.archive).resolve();report=(ROOT/args.report).resolve()
assert archive.is_relative_to(ROOT) and report.is_relative_to(ROOT)
report.parent.mkdir(parents=True,exist_ok=True)
area=report.parent/(report.stem+'-files');area.mkdir(exist_ok=True)
image=area/'root48.img';assert image.resolve().is_relative_to(ROOT/'work') and not image.is_symlink()
if image.exists():assert image.is_file()
with image.open('wb') as f:f.truncate(48*1024*1024)
sysroot=area/'sysroot';sysroot.mkdir(exist_ok=True);commands=[]
empty=area/'empty';empty.write_bytes(b'')
def quote(s):assert '\n' not in s and '\r' not in s and '"' not in s;return '"'+s+'"'
with tarfile.open(archive,'r:gz') as a:
 members=a.getmembers();names=set()
 for m in members:
  name=m.name.removeprefix('./').strip('/')
  assert not m.name.startswith('/') and '..' not in PurePosixPath(name).parts
  assert name not in names,(name,'duplicate');names.add(name)
  if not name:continue
  target='/'+name
  if m.isdir():commands.append('mkdir '+quote(target))
  elif m.isfile():
   commands.append('write '+quote(str(empty))+' '+quote(target));blocks=(m.size+4095)//4096
   if blocks:commands.append('fallocate '+quote(target)+f' 0 {blocks-1}')
   commands.append('set_inode_field '+quote(target)+f' size {m.size}')
  elif m.issym():commands.append('symlink '+quote(target)+' '+quote(m.linkname))
  elif m.islnk():commands.append('ln '+quote('/'+m.linkname.removeprefix('./'))+' '+quote(target))
  else:raise AssertionError(('unsupported archive type for allocation gate',m.name,m.type))
  if name.startswith('lib/') and '/modules/' not in name:
   p=sysroot/name;p.parent.mkdir(parents=True,exist_ok=True)
   if m.isfile():p.write_bytes(a.extractfile(m).read())
   elif m.issym():
    assert not PurePosixPath(m.linkname).is_absolute() and '..' not in PurePosixPath(m.linkname).parts
    if not p.is_symlink() and not p.exists():os.symlink(m.linkname,p)
 busybox=area/'busybox';busybox.write_bytes(a.extractfile('./bin/busybox').read());busybox.chmod(0o755)
mkfs=subprocess.run(['qemu-arm','-cpu','arm926','-L',str(sysroot),str(busybox),'mke2fs','-F','-b','4096',str(image)],capture_output=True,check=True)
commands.append('stats');batch=area/'allocate.commands';batch.write_text('\n'.join(commands)+'\n')
alloc=subprocess.run(['debugfs','-w','-f',str(batch),str(image)],capture_output=True)
log=(alloc.stdout+alloc.stderr).decode(errors='replace');(area/'allocation.log').write_text(log)
errors=[l for l in log.splitlines() if any(x in l.lower() for x in ['could not allocate','no space','file not found','file exists','usage:','not enough space'])]
free=[int(x)for x in re.findall(r'^Free blocks:\s+(\d+)',log,re.M)]
assert len(free)==1,'missing/ambiguous ext2 accounting result'
result={'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'root_partition_model_bytes':48*1024*1024,'block_size':4096,'minimum_free_bytes':4*1024*1024,'free_blocks':free[0],'free_bytes':free[0]*4096,'allocation_errors':errors,'passed':not errors and free[0]>=1024,'mke2fs_stdout':mkfs.stdout.decode(errors='replace'),'mke2fs_stderr':mkfs.stderr.decode(errors='replace'),'Sony_busybox_sha256':hashlib.sha256(busybox.read_bytes()).hexdigest(),'actual_device_geometry_read':False,'device_accessed':False}
report.write_text(json.dumps(result,indent=2))
print(json.dumps({'passed':result['passed'],'free_bytes':result['free_bytes'],'allocation_errors':errors}))
raise SystemExit(0 if result['passed'] else 2)
