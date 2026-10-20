"""Container corruption and untouched-component regression checks."""
import hashlib
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from upg import CIPHER, pack, unpack


def fixture():
    parts = [b'script', b'kernel-data', b'initrd-data', b'old-root']
    header = bytearray(struct.pack('<8sII', b'5dfeff3c', 4, 0) + bytes(32))
    output = bytearray(64)
    for index, data in enumerate(parts):
        struct.pack_into('<II', header, 16 + 8 * index, len(output), len(data))
        output += CIPHER.encrypt(data + bytes((-len(data)) % 8))
    output[16:64] = CIPHER.encrypt(header)
    output[:16] = hashlib.md5(output[16:]).digest()
    return bytes(output), parts


class ContainerTests(unittest.TestCase):
    def test_replace_root_preserves_first_three_components(self):
        original, parts = fixture()
        result = pack(original, b'a longer replacement root filesystem')
        self.assertEqual(unpack(result)[2], parts[:3] + [b'a longer replacement root filesystem'])

    def test_corrupt_outer_checksum_rejected(self):
        data, _ = fixture()
        with self.assertRaises(ValueError):
            unpack(data[:-1] + bytes([data[-1] ^ 1]))

    def test_overlapping_table_rejected_even_with_valid_md5(self):
        data, _ = fixture()
        data = bytearray(data)
        header = bytearray(CIPHER.decrypt(data[16:64]))
        struct.pack_into('<I', header, 24, 64)
        data[16:64] = CIPHER.encrypt(header)
        data[:16] = hashlib.md5(data[16:]).digest()
        with self.assertRaises(ValueError):
            unpack(data)


if __name__ == '__main__':
    unittest.main()
