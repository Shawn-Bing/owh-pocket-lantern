#!/usr/bin/env python3
"""Cross-build Windows x64 clients with Zig; build dashboard using installed Go."""
import argparse
import os
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
ap = argparse.ArgumentParser(description=__doc__)
ap.add_argument('--zig', required=True, help='Path to Zig executable; verified with 0.15.2')
ap.add_argument('--go', default='go')
args = ap.parse_args()
dist = root / 'dist'
dist.mkdir(exist_ok=True)
cache = root / 'build' / 'cache'
cache.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, ZIG_GLOBAL_CACHE_DIR=str(cache / 'zig'), ZIG_LOCAL_CACHE_DIR=str(cache / 'zig-local'), GOCACHE=str(cache / 'go'))
common = [args.zig, 'c++', '-target', 'x86_64-windows-gnu', '-std=c++20', '-O2', '-s', '-municode', '-Wl,--subsystem,windows', '-DUNICODE', '-D_UNICODE', '-I', str(root)]
for source, name, libs in [('main.cpp', 'overwatch-helper.exe', ['user32', 'gdi32', 'shell32', 'advapi32', 'comdlg32', 'winhttp', 'ole32']), ('demo.cpp', 'owh-demo.exe', ['user32', 'gdi32'])]:
    subprocess.run(common + [str(root / 'agent' / source), '-o', str(dist / name)] + ['-l' + x for x in libs], env=env, check=True)
for goos, arch, name in [('windows', 'amd64', 'owh-dashboard.exe'), ('linux', 'amd64', 'owh-dashboard-linux-amd64'), ('darwin', 'arm64', 'owh-dashboard-macos-arm64')]:
    subprocess.run([args.go, 'build', '-trimpath', '-ldflags=-s -w', '-o', str(dist / name), '.'], cwd=root / 'server', env=dict(env, GOOS=goos, GOARCH=arch, CGO_ENABLED='0'), check=True)
print('Built release files in', dist)
