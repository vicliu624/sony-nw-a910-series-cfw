"""Repository checks that do not download firmware or access a device."""
from pathlib import Path
import ast
import json
import re
import zlib
from upg import require, sha

root = Path(__file__).resolve().parents[1]
version = (root / 'VERSION').read_text().strip()
require(re.fullmatch(r'\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?', version), 'Invalid VERSION')
for name in ('README.md', 'README.zh-CN.md', 'README.ja.md'):
    text = (root / name).read_text(encoding='utf-8')
    require(all(link in text for link in ('README.md', 'README.zh-CN.md', 'README.ja.md')), 'Missing language links')
    require('0.1.0-alpha.1' not in text, 'Read version from VERSION, do not duplicate it')
    guide = {'README.md': 'TECHNICAL.md', 'README.zh-CN.md': 'TECHNICAL.zh-CN.md', 'README.ja.md': 'TECHNICAL.ja.md'}[name]
    guide_links = set(re.findall(r'\]\(docs/(TECHNICAL[^)]*)\)', text))
    require(guide_links == {guide}, 'README must link only its own technical-guide language')
    require('> **' in text, 'README is missing its visible flashing-risk notice')
    for image_path in re.findall(r'<img[^>]+src="([^"]+)"', text):
        require((root / image_path).is_file(), 'Missing README photo: ' + image_path)
for name in ('TECHNICAL.md', 'TECHNICAL.zh-CN.md', 'TECHNICAL.ja.md'):
    text = (root / 'docs' / name).read_text(encoding='utf-8')
    require(len(re.findall(r'^## \d+\.', text, re.M)) == 15, 'Incomplete technical guide')
    for language_id in range(11):
        require(f'| {language_id} | {language_id + 1} |' in text, 'Missing language ID mapping')
    for token in ('b8442c44', '5dfeff3c', '0x96294', '0x8e294', 'SWI 0x900000+nr', '106496', 'wording.py', 'update_resource_patch.py'):
        require(token in text.replace(',', ''), 'Missing technical handoff fact: ' + token)
for path in [root / n for n in ('README.md', 'README.zh-CN.md', 'README.ja.md')] + list((root / 'docs').glob('*.md')):
    for target in re.findall(r'\]\(([^)]+)\)', path.read_text(encoding='utf-8')):
        if '://' in target or target.startswith('#'):
            continue
        require((path.parent / target.split('#')[0]).exists(), 'Broken documentation link: ' + target)
for path in (root / 'scripts').glob('*.py'):
    ast.parse(path.read_text(encoding='utf-8'), filename=str(path))
manifest = json.loads((root / 'assets/patches/manifest.json').read_text())
require(len(manifest['patches']) == 5, 'Unexpected resource patch count')
for entry in manifest['patches']:
    data = (root / 'assets/patches' / entry['patch']).read_bytes()
    require(sha(data) == entry['patch_sha256'], 'Resource delta damaged')
    require(len(zlib.decompress(data)) >= entry['result_bytes'], 'Truncated delta')
print('Project layout, version, Python syntax and resource checks passed')
