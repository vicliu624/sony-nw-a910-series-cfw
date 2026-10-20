"""A910 UPG container helpers. No device access."""
import hashlib
import struct
from Crypto.Cipher import DES

CIPHER = DES.new(b'b8442c44', DES.MODE_ECB)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unpack(data):
    require(len(data) >= 64 and len(data) % 8 == 0, 'Invalid container size')
    require(data[:16] == hashlib.md5(data[16:]).digest(), 'UPG MD5 mismatch')
    header = CIPHER.decrypt(data[16:64])
    require(struct.unpack_from('<8sII', header) == (b'5dfeff3c', 4, 0), 'Wrong model header')
    entries = [struct.unpack_from('<II', header, 16 + i * 8) for i in range(4)]
    end = 64
    for offset, size in entries:
        require(offset >= end and offset % 8 == 0 and size > 0, 'Overlapping or invalid component')
        end = offset + ((size + 7) & ~7)
        require(end <= len(data), 'Truncated component')
    return header, entries, [CIPHER.decrypt(data[o:o + ((n + 7) & ~7)])[:n] for o, n in entries]


def pack(base, rootfs):
    header, entries, _ = unpack(base)
    header = bytearray(header)
    struct.pack_into('<II', header, 40, entries[3][0], len(rootfs))
    output = bytearray(base[:entries[3][0]])
    output[16:64] = CIPHER.encrypt(header)
    output += CIPHER.encrypt(rootfs + bytes((-len(rootfs)) % 8))
    output[:16] = hashlib.md5(output[16:]).digest()
    return bytes(output)


def replace_member(raw, member, payload):
    start = member.offset_data - 512
    header = bytearray(raw[start:member.offset_data])
    require(len(header) == 512, 'Unsupported tar header')
    header[124:136] = f'{len(payload):011o}\0'.encode()
    header[148:156] = b' ' * 8
    header[148:156] = f'{sum(header):06o}\0 '.encode()
    end = member.offset_data + ((member.size + 511) & ~511)
    return raw[:start] + header + payload + bytes((-len(payload)) % 512) + raw[end:]
