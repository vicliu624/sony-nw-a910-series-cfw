"""Update one existing source delta from a reviewed replacement file. No firmware/device writes."""
import argparse
import gzip
import io
import json
from pathlib import Path
import tarfile
import zlib
from upg import require, sha, unpack


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-upg', required=True)
    parser.add_argument('--path', required=True, help='Exact ./... path already in the patch manifest')
    parser.add_argument('--replacement', required=True)
    parser.add_argument('--apply', action='store_true', help='Write the reviewed delta and manifest; default is preview')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    manifest_path = root / 'assets/patches/manifest.json'
    manifest = json.loads(manifest_path.read_text())
    base = Path(args.original_upg).read_bytes()
    require(sha(base) == manifest['baseline_upg_sha256'], 'Original UPG mismatch')
    matches = [entry for entry in manifest['patches'] if entry['path'] == args.path]
    require(len(matches) == 1, 'Only one of the five existing patch paths may be updated')
    entry = matches[0]
    with tarfile.open(fileobj=io.BytesIO(gzip.decompress(unpack(base)[2][3]))) as archive:
        before = archive.extractfile(args.path).read()
    require(sha(before) == entry['original_sha256'], 'Original member mismatch')
    after = Path(args.replacement).read_bytes()
    require(after, 'Empty replacement rejected')
    size = max(len(before), len(after))
    delta = zlib.compress(bytes(a ^ b for a, b in zip(before.ljust(size, b'\0'), after.ljust(size, b'\0'))), 9)
    entry.update(result_sha256=sha(after), result_bytes=len(after), patch_sha256=sha(delta))
    if args.apply:
        (root / 'assets/patches' / entry['patch']).write_bytes(delta)
        manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps(dict(applied=args.apply, entry=entry), indent=2))


if __name__ == '__main__':
    main()
