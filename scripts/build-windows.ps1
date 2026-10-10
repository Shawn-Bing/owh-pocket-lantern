param(
    [string]$Generator = '',
    [string]$Go = '',
    [switch]$Race
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDir = Join-Path $projectRoot 'build/windows'
$distDir = Join-Path $projectRoot 'dist'
function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { throw 'CMake is required.' }
if (-not $Go) {
    $portableGo = Join-Path $projectRoot 'build/tools/go/bin/go.exe'
    if (Get-Command go -ErrorAction SilentlyContinue) { $Go = (Get-Command go).Source }
    elseif (Test-Path -LiteralPath $portableGo) { $Go = $portableGo }
    else { throw 'Go is required. Pass -Go C:\path\to\go.exe or install Go from https://go.dev/dl/.' }
}
if (-not (Test-Path -LiteralPath (Join-Path $buildDir 'CMakeCache.txt')) -and -not $Generator) {
    if (Get-Command cl -ErrorAction SilentlyContinue) { $Generator = 'NMake Makefiles' }
    elseif ((Get-Command g++ -ErrorAction SilentlyContinue) -and (Get-Command mingw32-make -ErrorAction SilentlyContinue)) {
        $Generator = 'MinGW Makefiles'
    }
}
$cmakeArgs = @('-S', $projectRoot, '-B', $buildDir, '-DCMAKE_BUILD_TYPE=Release')
if ($Generator) { $cmakeArgs += @('-G', $Generator) }
Invoke-Checked cmake $cmakeArgs
Invoke-Checked cmake @('--build', $buildDir, '--config', 'Release', '--parallel', '4')
Invoke-Checked ctest @('--test-dir', $buildDir, '-C', 'Release', '--output-on-failure')
New-Item -ItemType Directory -Force $distDir | Out-Null
$oldCache = $env:GOCACHE
try {
    $env:GOCACHE = Join-Path $projectRoot 'build/cache/go'
    Push-Location (Join-Path $projectRoot 'server')
    try {
        $testArgs = @('test', '-count=1')
        if ($Race) { $testArgs += '-race' }
        Invoke-Checked $Go ($testArgs + './...')
        Invoke-Checked $Go @('build', '-trimpath', '-ldflags=-s -w', '-o', (Join-Path $distDir 'owh-dashboard.exe'), '.')
    } finally { Pop-Location }
} finally { $env:GOCACHE = $oldCache }
foreach ($name in @('overwatch-helper.exe', 'owh-demo.exe', 'owh-recorder.exe', 'owh-teach.exe', 'owh-vision-calibrate.exe', 'owh-profile.exe')) {
    $source = Join-Path $buildDir $name
    if (-not (Test-Path -LiteralPath $source)) { $source = Join-Path $buildDir "Release/$name" }
    Copy-Item -LiteralPath $source -Destination (Join-Path $distDir $name) -Force
}
foreach ($name in @('start-demo.cmd', 'start-dashboard.cmd', 'diagnose.cmd', 'start-recording.cmd', 'stop-recording.cmd')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination $distDir -Force
}
New-Item -ItemType Directory -Force (Join-Path $distDir 'docs') | Out-Null
Get-ChildItem -LiteralPath (Join-Path $projectRoot 'docs') -Filter '*.md' | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $distDir 'docs') -Force
}
Copy-Item -LiteralPath (Join-Path $projectRoot 'README.md') -Destination $distDir -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'ocr-worker.ps1') -Destination $distDir -Force
# Recognition steps ship as data so they can be edited and shared without a rebuild.
New-Item -ItemType Directory -Force (Join-Path $distDir 'profiles') | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'profiles/cn-16x9.json') -Destination (Join-Path $distDir 'profiles') -Force
Write-Host "Windows build and tests complete: $distDir"
