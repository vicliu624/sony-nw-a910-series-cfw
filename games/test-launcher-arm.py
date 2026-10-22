"""Execute the built ARM workers with simulated LCD/input/USB only.

The simulated device boundary is explicit: these tests cannot qualify hardware.
Unlike shell-only dispatch tests, the real selector and Tetris ARM instructions
must draw, consume five-way events, return the expected code and release devices.
"""
import hashlib
import io
import json
from pathlib import Path
import struct

from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_INTR
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / 'work/game-apps'


def run_worker(binary, argument, keys=(), expected=0, failure=None, partial=False, usb=False):
    raw = (WORK / binary).read_bytes()
    elf = ELFFile(io.BytesIO(raw))
    symbols = {s.name: s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols()}
    u = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    segments = [s for s in elf.iter_segments() if s['p_type'] == 'PT_LOAD']
    start = min(s['p_vaddr'] for s in segments) & ~4095
    end = (max(s['p_vaddr'] + s['p_memsz'] for s in segments) + 4095) & ~4095
    u.mem_map(start, end - start)
    for s in segments:
        u.mem_write(s['p_vaddr'], s.data())
    u.mem_map(0x70000000, 0x20000)
    u.mem_map(0x60000000, 0x80000)
    u.mem_write(0x70000000, struct.pack('<3I', 0x70000100, 0x70000140, 0))
    u.mem_write(0x70000100, binary.encode() + b'\0')
    u.mem_write(0x70000140, argument.encode() + b'\0')
    u.reg_write(UC_ARM_REG_R0, 2)
    u.reg_write(UC_ARM_REG_R1, 0x70000000)
    u.reg_write(UC_ARM_REG_SP, 0x7001fff0)
    u.reg_write(UC_ARM_REG_LR, 0x70000200)
    clock = 0
    index = 0
    pending = b''
    delivered = False
    opened = set()
    grabbed = False
    mapped = False
    frames = []
    log = []
    paths = []

    def string(address):
        return bytes(u.mem_read(address, 256)).split(b'\0')[0].decode()

    def intr(_uc, _interrupt, _data):
        nonlocal clock, index, pending, delivered, grabbed, mapped
        instruction = struct.unpack('<I', u.mem_read(u.reg_read(UC_ARM_REG_PC)-4, 4))[0]
        immediate = instruction & 0xffffff
        assert 0x900000 <= immediate < 0x900120, hex(instruction)
        nr = immediate - 0x900000
        a, b, c = (u.reg_read(r) for r in (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2))
        result = 0
        if nr == 5:
            path = string(a)
            paths.append(path)
            if path.startswith('/tmp/a918-game-disk/APPS'):
                result = -2  # No apps installed; chooser must still work.
            else:
                assert path in ('/dev/fb/0', '/dev/input/event0', '/dev/usbconn'), path
                result = {'/dev/fb/0': 10, '/dev/input/event0': 11, '/dev/usbconn': 12}[path]
                if failure == path:
                    result = -2
                else:
                    opened.add(result)
        elif nr == 6:
            assert a in opened, a
            opened.remove(a)
        elif nr == 54:
            if failure == hex(b):
                result = -5
            elif b == 0x40044590:
                grabbed = bool(c)
            elif b == 0x80046701:
                u.mem_write(c, struct.pack('<I', int(usb)))
            elif b == 0x80044e04:
                assert struct.unpack('<2I', u.mem_read(c, 8)) == (0, 33)
            elif b in (0x40044e01, 0x40044e02):
                args = struct.unpack('<6I', u.mem_read(c, 24))
                assert args[:3] == (1, 0x800, 255) and args[3] in (0, 1), args
                if b == 0x40044e01:
                    frames.append(args[3])
            else:
                raise AssertionError(hex(b))
        elif nr == 192:
            if failure == 'mmap':
                result = -12
            else:
                assert b == 0x80000 and c == 3
                mapped = True
                result = 0x60000000
        elif nr == 91:
            assert mapped
            mapped = False
        elif nr == 78:
            u.mem_write(a, struct.pack('<2I', clock//1000, clock%1000*1000))
        elif nr == 162:
            clock += 250
            delivered = False
        elif nr == 3:
            assert a == 11, a
            if not pending and not delivered and index < len(keys):
                key = keys[index]
                index += 1
                delivered = True
                pending = struct.pack('<iiHHi', 0, 0, 1, key, 0) + struct.pack('<iiHHi', 0, 0, 1, key, 1)
            if pending:
                chunk = pending[:min(c, 5 if partial else c)]
                pending = pending[len(chunk):]
                u.mem_write(b, chunk)
                result = len(chunk)
            else:
                result = -11
        elif nr == 4:
            assert a in (1, 2), a
            log.append(bytes(u.mem_read(b, c)).decode(errors='replace'))
            result = c
        elif nr == 174:
            pass
        else:
            raise AssertionError(f'unexpected syscall {nr}')
        u.reg_write(UC_ARM_REG_R0, result & 0xffffffff)

    u.hook_add(UC_HOOK_INTR, intr)
    u.emu_start(symbols['run'], 0x70000200, count=60_000_000)
    assert u.reg_read(UC_ARM_REG_PC) == 0x70000200, 'worker did not return'
    rc = u.reg_read(UC_ARM_REG_R0)
    assert rc == expected, (binary, argument, failure, rc, expected, log)
    assert not opened and not grabbed and not mapped, (opened, grabbed, mapped)
    assert frames == [i % 2 for i in range(len(frames))]
    if argument == '--choose':
        assert not any(p.startswith('/tmp/') for p in paths), paths
    if not failure and not usb:
        assert frames, 'no screen presented'
    return dict(binary=binary, argument=argument, keys=list(keys), partial=partial,
                failure=failure, usb=usb, returncode=rc, frames=len(frames),
                all_devices_released=True, sha256=hashlib.sha256(raw).hexdigest())


cases = [
    run_worker('a918-boot', '--choose'),
    run_worker('a918-boot', '--choose', [0]),
    run_worker('a918-boot', '--choose', [4, 0], expected=90),
    run_worker('a918-boot', '--choose', [1, 0], expected=90, partial=True),
    run_worker('a918-boot', '--games', [4, 0]),
    run_worker('a918-boot', '--games', [0, 4, 0], partial=True),
    run_worker('tetris', '--games', [2, 1, 3, 4, 0]),
    run_worker('tetris', '--games', [2, 1, 3, 4, 0], partial=True),
    run_worker('a918-boot', '--choose', usb=True),
    run_worker('tetris', '--games', usb=True),
]
for failure, rc in [('/dev/fb/0', 77), ('0x80044e04', 78), ('mmap', 78),
                    ('/dev/input/event0', 79), ('0x40044590', 80),
                    ('/dev/usbconn', 0), ('0x80046701', 0),
                    ('0x40044e02', 78), ('0x40044e01', 78)]:
    cases.append(run_worker('a918-boot', '--choose', failure=failure, expected=rc))
report = dict(scope='actual built ARM workers; simulated LCD/input/USB; no hardware qualification', cases=cases)
(WORK/'launcher-arm-tests.json').write_text(json.dumps(report, indent=2)+'\n')
print(f'{len(cases)} built ARM chooser/Tetris worker cases PASS')
