"""Replay the original Sony kernel's ARM syscall decoder, not a modern ABI mock.
No syscalls are executed: stop at the resolved kernel entry. CP15 setup skipped.
"""
from pathlib import Path
import hashlib,json,struct,sys,tarfile,io
from elftools.elf.elffile import ELFFile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE,UC_HOOK_INTR
from unicorn.arm_const import UC_ARM_REG_R0
from unicorn.arm_const import UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_LR,UC_ARM_REG_PC,UC_ARM_REG_SP,UC_ARM_REG_CPSR
ROOT=Path(__file__).resolve().parents[1]
kernel=(ROOT/'work/decrypted-1.bin').read_bytes();base=0xc0008000
symbols={'vector_swi':0xc0022ae0,'sys_call_table':0xc0022ba4,'arm_syscall':0xc0028388}
assert hashlib.sha256(kernel).hexdigest()=='680bf2c4cae6be0d7d3f7028c150f45a3648a82feb0231772a9e60b33262faf4','Original kernel mismatch'
assert symbols['vector_swi']==0xc0022ae0 and symbols['sys_call_table']==0xc0022ba4
def decode(instruction,r7):
 u=Uc(UC_ARCH_ARM,UC_MODE_ARM);u.mem_map(base,0x400000);u.mem_write(base,kernel)
 u.mem_map(0x50000000,0x1000);u.mem_write(0x50000000,struct.pack('<I',instruction))
 u.mem_map(0x60000000,0x4000)
 u.reg_write(UC_ARM_REG_CPSR,0x13);u.reg_write(UC_ARM_REG_R8,0x10)
 u.reg_write(UC_ARM_REG_R7,r7);u.reg_write(UC_ARM_REG_LR,0x50000004);u.reg_write(UC_ARM_REG_SP,0x60001f80)
 entries={struct.unpack_from('<I',kernel,symbols['sys_call_table']-base+i*4)[0] for i in range(288)}
 reached=[]
 def hook(uc,pc,size,data):
  if pc==0xc0022b18:u.reg_write(UC_ARM_REG_PC,pc+4) # unrelated CP15 control write
  elif pc in entries or pc==symbols['arm_syscall']:
   reached.append(pc);u.emu_stop()
 u.hook_add(UC_HOOK_CODE,hook)
 u.emu_start(0xc0022b04,0,count=100)
 assert reached
 return {'selected_index':u.reg_read(UC_ARM_REG_R7),'entry':hex(reached[0]),'entry_names':[n for n,a in symbols.items() if a==reached[0]]}
cases=[]
for number in [1,2,3,4,5,6,10,11,15,19,21,37,38,39,42,54,55,57,78,91,99,114,118,141,162,173,174,192]:
 legacy=decode(0xef900000+number,999)
 expected=struct.unpack_from('<I',kernel,symbols['sys_call_table']-base+number*4)[0]
 assert legacy['entry']==hex(expected)
 modern=decode(0xef000000,number)
 assert modern['entry']==hex(symbols['arm_syscall']) and modern['selected_index']!=number
 cases.append({'requested':number,'OABI':legacy,'r7_svc0':modern,'passed':True})
report={'original_kernel_sha256':hashlib.sha256(kernel).hexdigest(),'decoder':'original kernel vector_swi ARM-mode instructions','CP15_setup_skipped':True,'hardware_syscalls_executed':False,'cases':cases,'conclusion':'ARM svc0 does not dispatch r7 on this kernel; ARM SWI 0x900000+nr selects the expected entry.'}
def first_call(binary):
 elf=ELFFile(io.BytesIO(binary));u=Uc(UC_ARCH_ARM,UC_MODE_ARM)
 segs=[s for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
 low=min(s['p_vaddr'] for s in segs)&~4095;high=(max(s['p_vaddr']+s['p_memsz'] for s in segs)+4095)&~4095
 u.mem_map(low,high-low)
 for s in segs:u.mem_write(s['p_vaddr'],s.data())
 u.mem_map(0x60000000,0x20000)
 u.mem_write(0x60010000,struct.pack('<4I',2,0x60010100,0x60010120,0))
 u.mem_write(0x60010100,b'a918-boot\0');u.mem_write(0x60010120,b'--boot\0');u.reg_write(UC_ARM_REG_SP,0x60010000)
 seen=[]
 def intr(uc,number,data):
  pc=u.reg_read(UC_ARM_REG_PC);instruction=struct.unpack('<I',u.mem_read(pc-4,4))[0]
  seen.append({'instruction':hex(instruction),'r7':u.reg_read(UC_ARM_REG_R7),'kernel_route':decode(instruction,u.reg_read(UC_ARM_REG_R7))});u.emu_stop()
 u.hook_add(UC_HOOK_INTR,intr);u.emu_start(elf['e_entry'],0,count=100000)
 assert len(seen)==1;return seen[0]
old_archive=ROOT/'work/game-r9-storage-final-3.bin'
new_file=ROOT/'work/game-apps/a918-boot'
if new_file.exists():
 new=new_file.read_bytes();new_first=first_call(new)
 expected_pipe=struct.unpack_from('<I',kernel,symbols['sys_call_table']-base+42*4)[0]
 assert new_first['kernel_route']['entry']==hex(expected_pipe)
 report.update({'new_program_sha256':hashlib.sha256(new).hexdigest(),'new_first_syscall':new_first,'first_pipe_syscall_fixed':True})
out=ROOT/'work/game-apps';out.mkdir(exist_ok=True)
(out/'kernel-decoder.json').write_text(json.dumps(report,indent=2))
print(f'{len(cases)} real-kernel decoder comparisons PASS; ARM svc0 incompatibility reproduced')
