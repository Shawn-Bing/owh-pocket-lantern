#!/usr/bin/env python3
"""Package existing release binaries and instructions; excludes local credentials."""
import argparse
import hashlib
import io
import tarfile
import zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=root / 'dist')
parser.add_argument('--binaries', type=Path, default=root / 'dist', help='Directory of built Windows binaries')
parser.add_argument('--windows-only', action='store_true', help='Do not require a Linux dashboard binary')
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
version = '0.5.12'
files = {name: (args.binaries / name).read_bytes() for name in (
    'overwatch-helper.exe', 'owh-demo.exe', 'owh-recorder.exe', 'owh-dashboard.exe',
    'owh-teach.exe', 'owh-vision-calibrate.exe', 'owh-profile.exe')}
for name in ('start-demo.cmd', 'start-dashboard.cmd', 'diagnose.cmd', 'start-recording.cmd', 'stop-recording.cmd'):
    files[name] = (root / 'scripts' / name).read_text().replace('\n', '\r\n').encode('ascii')
for path in sorted((root / 'docs').glob('*.md')):
    files['docs/' + path.name] = path.read_bytes()
files['README.md'] = (root / 'README.md').read_bytes()
files['ocr-worker.ps1'] = (root / 'scripts' / 'ocr-worker.ps1').read_bytes()
files['profiles/cn-16x9.json'] = (root / 'profiles' / 'cn-16x9.json').read_bytes()
files['SHA256SUMS.txt'] = ''.join(
    f'{hashlib.sha256(data).hexdigest()}  {name}\n' for name, data in sorted(files.items())
).encode('utf-8')
archive = args.output / f'OverwatchHelper-{version}-windows-x64.zip'
with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for name, data in files.items():
        z.writestr(f'OverwatchHelper-{version}/{name}', data)
if args.windows_only:
    print(f'{hashlib.sha256(archive.read_bytes()).hexdigest()}  {archive.name} ({archive.stat().st_size:,} bytes)')
    raise SystemExit(0)
server = root / 'dist' / 'owh-dashboard-linux-amd64'
linux = args.output / f'OverwatchHelper-{version}-dashboard-linux-x64.tar.gz'
with tarfile.open(linux, 'w:gz') as t:
    t.add(server, arcname=server.name)
    for name in ('docs/QUICKSTART.zh-CN.md', 'docs/ARCHITECTURE.md', 'docs/VALIDATION.md'):
        t.add(root / name, arcname=name)
    checksum = f'{hashlib.sha256(server.read_bytes()).hexdigest()}  {server.name}\n'.encode()
    item = tarfile.TarInfo('SHA256SUMS.txt')
    item.size = len(checksum)
    t.addfile(item, io.BytesIO(checksum))
for path in (archive, linux):
    print(f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name} ({path.stat().st_size:,} bytes)')
