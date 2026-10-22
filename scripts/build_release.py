"""Build and verify a local/CI release from the pinned Sony updater. No device I/O."""
import argparse
import gzip
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.request
import zipfile
import zlib

from upg import pack, replace_member, require, sha, unpack

ROOT = Path(__file__).resolve().parents[1]
os.chdir(ROOT)
WORK = ROOT / 'work/ci'
UPDATER_SHA = '6f745a8ff3c90b7abcd5ea185e6f51e4598492f6de5bdb3256599c6f43e39701'
URL = 'https://walkman.update.sony.net/fw/pc/A910/NW-A910_V1_03.exe'


def run(*args):
    subprocess.run(args, check=True, cwd=ROOT)


def write_json(path, data):
    path.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')


def prepare_updater():
    source = ROOT / 'firmware/original/NW-A910_V1_03.exe'
    source.parent.mkdir(parents=True, exist_ok=True)
    if not source.exists():
        temp = source.with_suffix('.download')
        with urllib.request.urlopen(URL, timeout=60) as response, temp.open('wb') as stream:
            shutil.copyfileobj(response, stream)
        require(sha(temp.read_bytes()) == UPDATER_SHA, 'Downloaded updater SHA-256 mismatch')
        temp.replace(source)
    require(sha(source.read_bytes()) == UPDATER_SHA, 'Sony updater SHA-256 mismatch')
    extracted = WORK / 'updater'
    extracted.mkdir(parents=True, exist_ok=True)
    run('7z', 'x', str(source), '-o' + str(extracted), '-y')
    expected = {'FWUpdater.exe', 'FWUpdaterCom.dll', 'FWUpdaterRes.dll', 'NW_A91X.UPG', 'NW-A91X.ini'}
    expected.update('license_' + language + '.txt' for language in ('cs', 'ct', 'en', 'fr', 'ge', 'it', 'jp', 'ko', 'sp', 'us'))
    require({p.name for p in extracted.iterdir()} == expected and all(p.is_file() for p in extracted.iterdir()),
            'Unexpected extracted updater files; use a clean work/ci/updater directory')
    return extracted


def firmware(base, language):
    manifest = json.loads((ROOT / 'assets/patches/manifest.json').read_text())
    require(sha(base) == manifest['baseline_upg_sha256'], 'Original UPG mismatch')
    _, _, parts = unpack(base)
    original = gzip.decompress(parts[3])
    raw = original
    replacements = {}
    for entry in manifest['patches']:
        with tarfile.open(fileobj=io.BytesIO(original)) as archive:
            before = archive.extractfile(entry['path']).read()
        require(sha(before) == entry['original_sha256'], 'Patch baseline mismatch')
        patch = (ROOT / 'assets/patches' / entry['patch']).read_bytes()
        require(sha(patch) == entry['patch_sha256'], 'Patch checksum mismatch')
        delta = zlib.decompress(patch)
        require(len(delta) == max(len(before), entry['result_bytes']), 'Patch length mismatch')
        after = bytes(a ^ b for a, b in zip(before.ljust(len(delta), b'\0'), delta))[:entry['result_bytes']]
        require(sha(after) == entry['result_sha256'], 'Patched resource mismatch')
        replacements[entry['path']] = after
        with tarfile.open(fileobj=io.BytesIO(raw)) as archive:
            raw = replace_member(raw, archive.getmember(entry['path']), after)
    # Rebuild each selected locale from the original wording, preserving all others.
    from wording import complete_pack
    app_path = './usr/local/bin/gemini-ICX1101'
    wording_path = './usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk'
    with tarfile.open(fileobj=io.BytesIO(original)) as archive:
        original_app = archive.extractfile(app_path).read()
        original_wording = archive.extractfile(wording_path).read()
    require(original_app[0x8e294:0x8e298] == bytes.fromhex('143090e5'), 'Language instruction baseline changed')
    app = bytearray(original_app)
    app[0x8e294:0x8e298] = bytes.fromhex('0030a0e3' if language == 'en' else '0630a0e3')
    completed, wording_report = complete_pack(original_wording, language)
    for path, data in ((app_path, bytes(app)), (wording_path, completed)):
        with tarfile.open(fileobj=io.BytesIO(raw)) as archive:
            raw = replace_member(raw, archive.getmember(path), data)
        replacements[path] = data
    write_json(WORK / ('wording-' + language + '.json'), wording_report)
    additions = {
        './usr/local/bin/a918-boot': (ROOT / 'work/game-apps/a918-boot').read_bytes(),
        './usr/local/bin/a918-boot-dispatch': (ROOT / 'games/boot-dispatch-apps.sh').read_bytes(),
    }
    with tarfile.open(fileobj=io.BytesIO(raw)) as archive:
        last = archive.getmembers()[-1]
        end = last.offset_data + ((last.size + 511) & ~511)
        template = archive.getmember('./usr/local/bin/gemini-ICX1101')
    raw = raw[:end]
    for name, data in additions.items():
        member = tarfile.TarInfo(name)
        member.mode = 0o755
        member.uid, member.gid, member.mtime = template.uid, template.gid, template.mtime
        member.uname, member.gname = template.uname, template.gname
        member.size = len(data)
        raw += member.tobuf(format=tarfile.USTAR_FORMAT) + data + bytes((-len(data)) % 512)
    raw += bytes(1024)
    raw += bytes((-len(raw)) % 10240)
    with tarfile.open(fileobj=io.BytesIO(original)) as before, tarfile.open(fileobj=io.BytesIO(raw)) as after:
        require(after.getnames() == before.getnames() + list(additions), 'Unexpected rootfs members')
        for member in before.getmembers():
            updated = after.getmember(member.name)
            for field in ('mode', 'uid', 'gid', 'mtime', 'type', 'linkname', 'uname', 'gname', 'devmajor', 'devminor'):
                require(getattr(member, field) == getattr(updated, field), 'Metadata changed: ' + member.name)
            if member.isfile():
                expected = replacements.get(member.name, before.extractfile(member).read())
                require(after.extractfile(updated).read() == expected, 'Unexpected file change: ' + member.name)
    compressed = gzip.compress(raw, compresslevel=9, mtime=0)
    archive_path = WORK / 'rootfs.tar.gz'
    archive_path.write_bytes(compressed)
    run(sys.executable, 'scripts/check-rootfs-capacity.py', str(archive_path), str(WORK / 'capacity.json'))
    output = pack(base, compressed)
    require(unpack(output)[2] == parts[:3] + [compressed], 'Container roundtrip failed')
    candidate = WORK / 'candidate.UPG'
    candidate.write_bytes(output)
    run('tools/rockbox-upgtools/upgtool', '--no-color', '-e', '-k', 'b8442c44', '-s', '5dfeff3c', '-o', str(WORK / 'decoded-'), str(candidate))
    for index, expected in enumerate(parts[:3] + [compressed]):
        require((WORK / f'decoded-{index}.bin').read_bytes() == expected, 'Independent Rockbox decode differs')
    return output, list(replacements) + list(additions)


def main():
    require(not sys.flags.optimize, 'Python -O/PYTHONOPTIMIZE is forbidden')
    parser = argparse.ArgumentParser()
    parser.add_argument('--language', choices=['en', 'zh-CN'], default='en')
    parser.add_argument('--version', default=(ROOT / 'VERSION').read_text().strip())
    args = parser.parse_args()
    require(re.fullmatch(r'\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?', args.version), 'Invalid release version')
    require(args.version == (ROOT / 'VERSION').read_text().strip(), 'Tag/version differs from VERSION')
    WORK.mkdir(parents=True, exist_ok=True)
    updater = prepare_updater()
    run('sh', 'games/build-apps-linux.sh')
    # Test fixtures contain executable programs only. Commercial ROMs never enter CI.
    apps = WORK / 'apps'
    os.environ['A918_APP_FIXTURES'] = str(apps)
    for app_id, title in [('tetris', 'TETRIS'), ('tamagotchi-id', 'TAMAGOTCHI ID')]:
        folder = apps / app_id
        folder.mkdir(parents=True, exist_ok=True)
        data = (ROOT / 'work/game-apps' / app_id).read_bytes()
        (folder / 'app').write_bytes(data)
        (folder / 'app.ini').write_text(f'api=1\nid={app_id}\nname={title}\nversion={args.version}\nexec=app\nbytes={len(data)}\ncrc32={zlib.crc32(data):08x}\n', encoding='ascii')
    run(sys.executable, 'games/test-disk-apps.py', '--without-rom')
    run(sys.executable, 'games/test-app-dispatch.py')
    run(sys.executable, 'games/test-launcher-arm.py')
    base = (updater / 'NW_A91X.UPG').read_bytes()
    (ROOT / 'work/decrypted-1.bin').write_bytes(unpack(base)[2][1])
    run(sys.executable, 'games/check-apps-kernel-abi.py')
    run(sys.executable, 'games/check-apps-elf-loader.py')
    output, changed = firmware(base, args.language)
    destination = ROOT / 'dist' / args.version / args.language
    require(not destination.exists(), 'Output already exists; choose a clean dist directory')
    stage = WORK / ('package-' + args.language)
    require(not stage.exists(), 'Staging directory exists; remove work/ci/package before rebuilding')
    stage.mkdir()
    installer = stage / 'updater'
    shutil.copytree(updater, installer)
    (installer / 'NW_A91X.UPG').write_bytes(output)
    for app_id in ('tetris', 'tamagotchi-id'):
        folder = stage / 'WALKMAN-files/APPS' / app_id
        folder.mkdir(parents=True)
        for filename in ('app', 'app.ini'):
            shutil.copy2(apps / app_id / filename, folder / filename)
    for filename in ('README.md', 'README.zh-CN.md', 'README.ja.md', 'THIRD_PARTY.md'):
        shutil.copy2(ROOT / filename, stage / filename)
    shutil.copytree(ROOT / 'docs', stage / 'docs')
    shutil.copytree(ROOT / 'assets/images', stage / 'assets/images')
    for directory in ('games', 'scripts', 'assets/patches', 'assets/images', 'translations', 'tests', 'third_party', '.github'):
        shutil.copytree(ROOT / directory, stage / 'source' / directory,
                        ignore=shutil.ignore_patterns('__pycache__', '*.pyc', '*.o', 'upgtool'))
    shutil.copytree(ROOT / 'docs', stage / 'source/docs')
    for name in ('VERSION', 'requirements.txt', 'THIRD_PARTY.md', 'README.md', 'README.zh-CN.md', 'README.ja.md', 'AGENTS.md', 'CHANGELOG.md', '.gitignore', '.gitattributes'):
        shutil.copy2(ROOT / name, stage / 'source' / name)
    for name in ('app-tests.json', 'dispatch-tests.json', 'launcher-arm-tests.json', 'elf-loader-tests.json', 'kernel-decoder.json'):
        shutil.copy2(ROOT / 'work/game-apps' / name, stage / name)
    shutil.copy2(WORK / 'capacity.json', stage / 'capacity.json')
    shutil.copy2(WORK / ('wording-' + args.language + '.json'), stage / 'wording.json')
    report = dict(version=args.version, language=args.language, language_resource_id=0 if args.language == 'en' else 6, firmware_sha256=sha(output), firmware_bytes=len(output), changed_system_files=changed,
                  stock_upgrade_script_kernel_initrd_unchanged=True, hardware_tested=False,
                  rom_tests_run=False, roms_included=False, actual_device_geometry_read=False)
    write_json(stage / 'verification.json', report)
    forbidden = {'rom.bin', 'new.sav', 'state.sav', 'state.bak'}
    require(not any(p.name in forbidden for p in stage.rglob('*')), 'Private ROM/save entered package')
    files = sorted(p for p in stage.rglob('*') if p.is_file())
    (stage / 'SHA256SUMS.txt').write_text(''.join(f'{sha(p.read_bytes())}  {p.relative_to(stage).as_posix()}\n' for p in files))
    destination.mkdir(parents=True)
    bundle = destination / f'A918-{args.version}-{args.language}-experimental.zip'
    with zipfile.ZipFile(bundle, 'w', zipfile.ZIP_DEFLATED) as archive:
        for path in files + [stage / 'SHA256SUMS.txt']:
            archive.write(path, path.relative_to(stage).as_posix())
    with zipfile.ZipFile(bundle) as archive:
        require(archive.testzip() is None, 'ZIP damaged')
        for path in files:
            require(archive.read(path.relative_to(stage).as_posix()) == path.read_bytes(), 'ZIP content mismatch')
    shutil.copy2(stage / 'verification.json', destination / f'verification-{args.language}.json')
    (destination / f'SHA256SUMS-{args.language}.txt').write_text(f'{sha(bundle.read_bytes())}  {bundle.name}\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
