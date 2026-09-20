"""Package the standalone passive recorder, without recordings or configuration."""
from pathlib import Path
import hashlib
import zipfile

root = Path(__file__).resolve().parents[1]
version = '0.1.1'
files = {'owh-recorder.exe': (root/'dist/owh-recorder.exe').read_bytes()}
for name in ('start-recording.cmd', 'stop-recording.cmd'):
    files[name] = (root/'scripts'/name).read_text(encoding='ascii').replace('\n', '\r\n').encode('ascii')
files['README.zh-CN.md'] = (root/'docs/RECORDER.zh-CN.md').read_bytes()
files['SHA256SUMS.txt'] = ''.join(f'{hashlib.sha256(v).hexdigest()}  {k}\n' for k,v in sorted(files.items())).encode()
output = root/f'dist/OverwatchHelper-Recorder-{version}-windows-x64.zip'
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
    for name,content in files.items():
        archive.writestr(f'OverwatchHelper-Recorder-{version}/{name}',content)
print(f'{output.name}: {output.stat().st_size:,} bytes, SHA256 {hashlib.sha256(output.read_bytes()).hexdigest()}')
