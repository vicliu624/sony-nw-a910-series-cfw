"""DWR parser and EN/ZH completion, promoted from the original verified research.
Produces a wording resource only, never a firmware or device write.
"""
from pathlib import Path
import argparse
import csv
import gzip
import io
import json
import struct
import sys
import tarfile
import textwrap
import unicodedata
from upg import unpack, require, sha

def parse_dwr(data):
    locale, count, length_pool, text_pool = struct.unpack_from('<4I', data)
    assert length_pool == 16 + count * 8
    assert length_pool <= text_pool <= len(data)
    records = []
    for index in range(count):
        lines, secondary, length_index, text_index, metadata = struct.unpack_from('<bbHHH', data, 16+index*8)
        assert 0 <= lines <= 127
        assert length_pool + 2*(length_index+lines) <= text_pool
        lengths = struct.unpack_from(f'<{lines}H', data, length_pool+2*length_index)
        position = text_pool + 2*text_index
        texts = []
        for length in lengths:
            assert length > 0 and position + 2*length <= len(data), (locale, index)
            raw = data[position:position+length*2]
            assert raw[-2:] == b'\0\0', (locale, index, length)
            texts.append(raw[:-2].decode('utf-16-le'))
            position += length*2
        records.append({'id': index, 'lines': texts, 'text': '\n'.join(texts),
                        'line_count': lines, 'secondary': secondary, 'metadata': metadata,
                        'length_index': length_index, 'text_index': text_index})
    return {'locale': locale, 'count': count, 'records': records}

def wrap_text(text, language):
    if '\n' in text:
        return [line for paragraph in text.split('\n') for line in wrap_text(paragraph, language)]
    if language == 'english':
        return textwrap.wrap(text, width=20, break_long_words=False, break_on_hyphens=False) or ['']
    lines, line, width = [], '', 0
    for char in text:
        weight = 2 if unicodedata.east_asian_width(char) in 'WF' else 1
        if width + weight > 20:
            lines.append(line)
            line, width = '', 0
        line += char
        width += weight
    return lines + [line]

def fill_dwr(original, japanese, translations, language):
    parsed = parse_dwr(original)
    _, count, length_pool, text_pool = struct.unpack_from('<4I', original)
    records = bytearray(original[16:length_pool])
    lengths = bytearray(original[length_pool:text_pool])
    text = bytearray(original[text_pool:])
    changes, overflow = [], []
    for jp, old in zip(japanese['records'], parsed['records']):
        if not jp['text'].strip() or old['text'].strip():
            continue
        translation = translations[jp['id']][language].replace('\\n', '\n')
        assert translation.strip(), (jp['id'], language)
        lines = wrap_text(translation, language)
        if len(lines) > jp['line_count']:
            overflow.append({'id': jp['id'], 'language': language, 'lines': lines, 'max': jp['line_count']})
        length_index, text_index = len(lengths)//2, len(text)//2
        assert max(length_index, text_index) <= 65535
        for line in lines:
            raw = (line+'\0').encode('utf-16-le')
            lengths += struct.pack('<H', len(raw)//2)
            text += raw
        # Preserve Japanese dynamic insertion line for originally absent records.
        secondary = jp['secondary']
        metadata = old['metadata']
        struct.pack_into('<bbHHH', records, jp['id']*8, len(lines), secondary, length_index, text_index, metadata)
        changes.append({'id': jp['id'], 'japanese': jp['text'], 'translation': '\n'.join(lines), 'lines': len(lines)})
    lengths += bytes((-len(lengths)) % 16)
    rebuilt = struct.pack('<4I', parsed['locale'], count, length_pool, length_pool+len(lengths)) + records + lengths + text
    rebuilt += bytes((-len(rebuilt)) % 16)
    verified = parse_dwr(rebuilt)
    for jp, old, new in zip(japanese['records'], parsed['records'], verified['records']):
        if old['text'].strip() or not jp['text'].strip():
            assert old == new, ('Original record changed', old['id'])
        else:
            assert new['text'].strip(), ('Missing record', new['id'])
    return rebuilt, changes, overflow

def complete_pack(original, language_code):
    require(language_code in ('en','zh-CN'), 'Unsupported completion language')
    root=Path(__file__).resolve().parents[1]
    count=struct.unpack_from('<I',original,4)[0]
    require(count==12,'Unexpected DPK section count')
    pairs=[struct.unpack_from('<QQ',original,16+16*i) for i in range(count)]
    blocks=[original[start:(pairs[i+1][1] if i+1<count else len(original))] for i,(_,start) in enumerate(pairs)]
    jp=parse_dwr(blocks[2])
    with (root/'translations/missing.tsv').open(encoding='utf-8',newline='') as stream:
        translations={int(row['id']):row for row in csv.DictReader(stream,delimiter='\t')}
    language,section=('english',1) if language_code=='en' else ('simplified_chinese',7)
    rebuilt,changes,overflow=fill_dwr(blocks[section],jp,translations,language)
    require(not overflow,'Shorten translations that exceed Japanese line counts: '+str(overflow))
    require(len(changes)==(288 if language_code=='en' else 289),'Unexpected missing wording count')
    blocks[section]=rebuilt
    cursor=16+16*count;index=bytearray()
    for (key,_),block in zip(pairs,blocks):
        index+=struct.pack('<QQ',key,cursor);cursor+=len(block)
    result=struct.pack('<4I',cursor-(16+16*count),count,0,0)+index+b''.join(blocks)
    return result, {'language':language_code,'filled':len(changes),'missing_after':0,'sha256':sha(result),'bytes':len(result)}


def main():
    require(not sys.flags.optimize, 'Do not run with Python -O')
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--upg',required=True)
    ap.add_argument('--language',choices=['en','zh-CN'],required=True)
    ap.add_argument('--output',required=True)
    args=ap.parse_args()
    root=Path(__file__).resolve().parents[1]
    data=Path(args.upg).read_bytes()
    require(sha(data)=='cfbcc372e2a18f11c12a080c9e5f039333e09175f5db03ac841a4227d3896b32','Requires original A910 1.03 UPG')
    with tarfile.open(fileobj=io.BytesIO(gzip.decompress(unpack(data)[2][3]))) as archive:
        original=archive.extractfile('./usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk').read()
    result,report=complete_pack(original,args.language)
    output=Path(args.output).resolve()
    require(output.is_relative_to((root/'work').resolve()),'Output must be under project work/')
    output.parent.mkdir(parents=True,exist_ok=True);output.write_bytes(result)
    output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Wording resource generated:',output,'filled:',report['filled'])

if __name__=='__main__':
    main()
