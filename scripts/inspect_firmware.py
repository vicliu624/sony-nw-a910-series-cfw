"""Export A910 components and wording for offline research; never execute them."""
import argparse
import gzip
import io
import json
from pathlib import Path
import struct
import tarfile
from upg import unpack, require, sha
from wording import parse_dwr


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--upg', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = Path(args.output).resolve()
    require(output.is_relative_to((root / 'work').resolve()) and not output.exists(),
            'Choose a new output directory under project work/')
    data = Path(args.upg).read_bytes()
    _, entries, parts = unpack(data)
    output.mkdir(parents=True)
    for index, part in enumerate(parts):
        (output / f'component-{index}.bin').write_bytes(part)
    with tarfile.open(fileobj=io.BytesIO(gzip.decompress(parts[3]))) as archive:
        inventory = [{key: value.hex() if isinstance(value, bytes) else value
                      for key, value in member.get_info().items()} for member in archive.getmembers()]
        # Fixed names only: do not extract arbitrary archive paths onto the host.
        paths = {
            'gemini-ICX1101': './usr/local/bin/gemini-ICX1101',
            'init-gemini.sh': './etc/init.d/gemini',
            'wording.dpk': './usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk',
            'dui_tex.dpk': './usr/local/share/gemini/res/dui_tex/dui_tex.dpk',
            'ICX1101_home_anim.dpk': './usr/local/share/gemini/res/dui_tex/ICX1101_home_anim.dpk',
        }
        for name, path in paths.items():
            (output / name).write_bytes(archive.extractfile(path).read())
    wording = (output / 'wording.dpk').read_bytes()
    count = struct.unpack_from('<I', wording, 4)[0]
    require(count == 12, 'Unexpected wording DPK')
    index = [struct.unpack_from('<QQ', wording, 16 + i * 16) for i in range(count)]
    locales = {}
    for i, (key, start) in enumerate(index):
        end = index[i + 1][1] if i + 1 < count else len(wording)
        section = wording[start:end]
        (output / f'section-{i:02}.dwr').write_bytes(section)
        locales[str(i)] = parse_dwr(section)
    report = dict(upg_sha256=sha(data), components=[dict(index=i, offset=entries[i][0], bytes=len(part), sha256=sha(part)) for i, part in enumerate(parts)],
                  selector_bytes=(output / 'gemini-ICX1101').read_bytes()[0x8e294:0x8e298].hex(),
                  rootfs_inventory=inventory, wording=locales, device_accessed=False)
    (output / 'inspection.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Exported offline evidence:', output)


if __name__ == '__main__':
    main()
