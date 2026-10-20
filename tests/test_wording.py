"""Regression boundaries for localization: retain existing text and reserved IDs."""
import struct
import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from wording import fill_dwr, parse_dwr


def dwr(locale, text, secondary=0):
    lines = 1 if text else 0
    record = struct.pack('<bbHHH', lines, secondary, 0, 0, 7)
    lengths = struct.pack('<H', len(text.encode('utf-16-le')) // 2 + 1) if text else b''
    encoded = (text + '\0').encode('utf-16-le') if text else b''
    return struct.pack('<4I', locale, 1, 24, 24 + len(lengths)) + record + lengths + encoded


class WordingTests(unittest.TestCase):
    def test_fill_missing_preserves_dynamic_field_and_target_metadata(self):
        japanese = parse_dwr(dwr(1, '録音', secondary=1))
        result, changes, overflow = fill_dwr(dwr(0, ''), japanese, {0: {'english': 'Record'}}, 'english')
        record = parse_dwr(result)['records'][0]
        self.assertEqual((record['text'], record['secondary'], record['metadata']), ('Record', 1, 7))
        self.assertEqual((len(changes), overflow), (1, []))

    def test_existing_translation_is_not_replaced(self):
        original = dwr(0, 'Original')
        result, changes, _ = fill_dwr(original, parse_dwr(dwr(1, '録音')), {}, 'english')
        self.assertEqual(parse_dwr(result)['records'], parse_dwr(original)['records'])
        self.assertEqual(changes, [])

    def test_japanese_reserved_blank_stays_blank(self):
        result, changes, _ = fill_dwr(dwr(0, ''), parse_dwr(dwr(1, '')), {}, 'english')
        self.assertEqual(parse_dwr(result)['records'][0]['text'], '')
        self.assertEqual(changes, [])

    def test_missing_utf16_terminator_is_rejected(self):
        damaged = dwr(0, 'Record')[:-2] + b'xx'
        with self.assertRaises(AssertionError):
            parse_dwr(damaged)

    def test_too_many_translation_lines_is_reported(self):
        _, _, overflow = fill_dwr(dwr(0, ''), parse_dwr(dwr(1, '録音')), {0: {'english': 'First\nSecond'}}, 'english')
        self.assertEqual(len(overflow), 1)


if __name__ == '__main__':
    unittest.main()
