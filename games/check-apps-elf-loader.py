"""Replay the original kernel's ELF admission gate, not a modern ELF loader.

Stop at the first program-header read: no real kernel allocation, mapping,
filesystem access or application execution. Allocation hooks use ordinary
emulator memory. This checks admission only, not complete process startup.
"""
from pathlib import Path
import hashlib, json, struct
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC, UC_ARM_REG_CPSR

r = Path(__file__).resolve().parents[1]
kernel = (r/'work/decrypted-1.bin').read_bytes()
assert hashlib.sha256(kernel).hexdigest() == '680bf2c4cae6be0d7d3f7028c150f45a3648a82feb0231772a9e60b33262faf4'
symbols = dict(load_elf_binary=0xc00aa6c8, kmem_cache_alloc=0xc006b2e0,
               __kmalloc=0xc006b254, kfree=0xc006ba68, kernel_read=0xc008b1f0)
base = 0xc0008000


def gate(label, header, expected):
    u = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    u.mem_map(base, 0x400000); u.mem_write(base, kernel)
    u.mem_map(0x60000000, 0x100000)
    bprm, file, fops, stack = 0x60010000, 0x60011000, 0x60012000, 0x6007f000
    u.mem_write(bprm, bytes(header[:128]))
    u.mem_write(bprm + 0x10c, struct.pack('<I', file))
    u.mem_write(file + 0x10, struct.pack('<I', fops))
    u.mem_write(fops + 0x2c, struct.pack('<I', 1))
    u.reg_write(UC_ARM_REG_CPSR, 0x13)
    u.reg_write(UC_ARM_REG_R0, bprm); u.reg_write(UC_ARM_REG_R1, 0x60020000)
    u.reg_write(UC_ARM_REG_SP, stack); u.reg_write(UC_ARM_REG_LR, 0x60030000)
    allocations = 0
    accepted = False

    def hook(_u, pc, _size, _data):
        nonlocal allocations, accepted
        if pc in (symbols['kmem_cache_alloc'], symbols['__kmalloc']):
            u.reg_write(UC_ARM_REG_R0, 0x60040000 + allocations * 0x1000)
            allocations += 1
            u.reg_write(UC_ARM_REG_PC, u.reg_read(UC_ARM_REG_LR))
        elif pc == symbols['kfree']:
            u.reg_write(UC_ARM_REG_PC, u.reg_read(UC_ARM_REG_LR))
        elif pc == symbols['kernel_read']:
            accepted = True
            u.emu_stop()
        elif pc == 0x60030000:
            u.emu_stop()

    u.hook_add(UC_HOOK_CODE, hook)
    u.emu_start(symbols['load_elf_binary'], 0, count=100000)
    assert accepted == expected, (label, hex(u.reg_read(UC_ARM_REG_PC)),hex(u.reg_read(UC_ARM_REG_R0)))
    if not expected:
        assert u.reg_read(UC_ARM_REG_PC) == 0x60030000
        assert u.reg_read(UC_ARM_REG_R0) == 0xfffffff8
    return dict(case=label, admitted_to_program_header_read=accepted, expected=expected)


cases = []
for name in ('a918-boot','tetris','tamagotchi-id'):
    header = (r/'work/game-apps'/name).read_bytes()[:128]
    cases.append(gate(name, header, True))
    for label, offset, data in [('bad magic',0,b'NOPE'),('wrong architecture',18,b'\x03\0'),
                                ('wrong program header size',42,b'\x1f\0'),
                                ('no program headers',44,b'\0\0')]:
        invalid = bytearray(header); invalid[offset:offset+len(data)] = data
        cases.append(gate(name+' / '+label, invalid, False))
report = dict(scope='original kernel instructions, ELF header admission only; no full exec or hardware',
              original_kernel_sha256=hashlib.sha256(kernel).hexdigest(),cases=cases)
(r/'work/game-apps/elf-loader-tests.json').write_text(json.dumps(report,indent=2)+'\n')
print(f'{len(cases)} original-kernel ELF admission checks PASS')
