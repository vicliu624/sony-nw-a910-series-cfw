"""Remove explicit regenerable work products; preserve historical evidence first.

Dry run by default. Must run on Linux/WSL to handle Linux symlinks correctly.
No real disks, releases or firmware inputs are removed.
"""
import argparse
import json
from pathlib import Path
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / 'work'
ARCHIVE = ROOT / 'archive/local/recovery'
GENERATED = {
    'boot-abi-diagnosis', 'chiikawa-review', 'chiikawa-theme', 'completed-wording',
    'fm-native-analysis', 'game-boot', 'game-browser', 'game-build', 'game-color',
    'game-color-menu', 'game-color-perf', 'game-delivery', 'game-external',
    'game-inline', 'game-p1', 'game-preview', 'install-english',
    'install-english-chiikawa-b1-r3', 'install-english-chiikawa-dark-r4',
    'install-english-chiikawa-dark-r5', 'install-english-chiikawa-r6-game-alpha1',
    'install-english-chiikawa-r6-game-alpha2', 'install-english-r2',
    'install-simplified-chinese-r2', 'restore-original-1.03', 'rootfs',
    'rootfs-usage', 'stock-trim-audit', 'texture-inspection', 'updater-1.03',
    'watchman-root', 'wording', 'wording-comparison',
}


def size(path):
    if path.is_symlink():
        return 0
    if path.is_file():
        return path.stat().st_size
    return sum(p.stat().st_size for p in path.rglob('*') if not p.is_symlink() and p.is_file())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    if sys.platform != 'linux':
        raise SystemExit('Use Linux/WSL, not Windows symlink handling')
    actions = []
    ARCHIVE.mkdir(parents=True, exist_ok=True)
    # These contain unique evidence or private inputs and are never deletion candidates.
    preserved = ['device-backup-1.00', 'device-backup-preflash', 'recovery-r12', 'game-research']
    for name in preserved:
        path = WORK / name
        if path.exists():
            target = ARCHIVE / name
            if target.exists():
                raise SystemExit('Archive already exists: ' + str(target))
            actions.append({'action': 'archive', 'path': str(path.relative_to(ROOT)), 'bytes': size(path)})
            if args.apply:
                shutil.move(str(path), str(target))
    seed = WORK / 'game-color-menu/new-game.sav'
    if seed.exists():
        actions.append({'action': 'preserve-private-seed', 'path': str(seed.relative_to(ROOT)), 'bytes': seed.stat().st_size})
        if args.apply:
            shutil.copy2(seed, ARCHIVE / 'id-new-game.sav')
    for path in sorted(WORK.iterdir()):
        # Keep current CI evidence and Python environments. Archive unknown files.
        if path.name in {'ci', 'game-apps', 'ci-venv', 'venv', 'kernel-tools-venv', 'lib'}:
            continue
        if path.name in preserved:
            continue
        if path.is_symlink():
            raise SystemExit('Unexpected work-root symlink: ' + str(path))
        if path.is_dir() and path.name not in GENERATED:
            continue
        action = 'delete-generated' if path.is_dir() or path.suffix in {'.bin', '.UPG', '.zip'} else 'archive'
        # Every recursive deletion is resolved and checked against this exact work root.
        target = path.resolve()
        if not target.is_relative_to(WORK.resolve()) or target == WORK.resolve():
            raise SystemExit('Unsafe cleanup target: ' + str(target))
        actions.append({'action': action, 'path': str(path.relative_to(ROOT)), 'bytes': size(path)})
        if args.apply:
            if action == 'archive':
                shutil.move(str(path), str(ARCHIVE / path.name))
            elif path.is_dir():
                for evidence in path.rglob('*'):
                    if not evidence.is_symlink() and evidence.is_file() and evidence.suffix in {'.json', '.log', '.md'}:
                        saved = ARCHIVE / 'reports' / path.name / evidence.relative_to(path)
                        saved.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(evidence, saved)
                shutil.rmtree(path)
            else:
                path.unlink()
    result = {'applied': args.apply, 'deleted_bytes': sum(a['bytes'] for a in actions if a['action'] == 'delete-generated'), 'actions': actions}
    if args.apply:
        (ROOT / 'archive/local/cleanup-report.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
