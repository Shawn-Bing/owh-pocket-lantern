#!/usr/bin/env python3
"""Reproduce static unpacking of this exact sample without executing it.

Requires pefile and a separately installed official UPX binary.
Writes only to --out; source bytes are never modified.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

import pefile

EXPECTED_SHA256 = 'fdba49702efb71deacfdb47459ec713be69fe004bf8e04d5bd4150a8f9ed8296'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('sample', type=Path)
    ap.add_argument('--upx', required=True, type=Path)
    ap.add_argument('--out', required=True, type=Path)
    args = ap.parse_args()
    source = args.sample.resolve()
    original = source.read_bytes()
    digest = hashlib.sha256(original).hexdigest()
    if digest != EXPECTED_SHA256:
        ap.error('Input hash differs from the analyzed sample. Refusing automatic header patching.')
    args.out.mkdir(parents=True, exist_ok=True)
    restored = args.out / 'ow.section-restored.bin'
    unpacked = args.out / 'ow.unpacked.bin'
    if restored.exists() or unpacked.exists():
        ap.error('Output binaries already exist; choose a fresh output directory.')
    data = bytearray(original)
    pe = pefile.PE(data=data)
    changes = []
    for section in pe.sections:
        if section.Name in (b'XXX0\0\0\0\0', b'XXX1\0\0\0\0'):
            offset = section.get_file_offset()
            old = bytes(data[offset:offset + 8])
            new = b'UPX' + old[3:]
            data[offset:offset + 8] = new
            changes.append({'offset': offset, 'old_hex': old.hex(), 'new_hex': new.hex()})
    if len(changes) != 2:
        ap.error('Expected exactly two renamed UPX sections.')
    restored.write_bytes(data)
    (args.out / 'section-restoration.json').write_text(json.dumps(changes, indent=2) + '\n')
    result = subprocess.run([str(args.upx.resolve()), '-d', str(restored.resolve()), '-o', str(unpacked.resolve())], capture_output=True, text=True)
    (args.out / 'unpack.log').write_text(result.stdout + result.stderr)
    if result.returncode:
        raise SystemExit('UPX failed; see unpack.log')
    if hashlib.sha256(source.read_bytes()).hexdigest() != digest:
        raise SystemExit('Source hash changed during analysis; investigate external modification.')
    print(json.dumps({'source_sha256': digest, 'unpacked_sha256': hashlib.sha256(unpacked.read_bytes()).hexdigest(), 'unpacked_bytes': unpacked.stat().st_size}, indent=2))


if __name__ == '__main__':
    main()
