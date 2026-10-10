param(
  [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\build\windows')
)
$ErrorActionPreference = 'Stop'
$sensorCompiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (-not (Test-Path -LiteralPath $sensorCompiler)) { throw '.NET Framework 4.8 compiler is required; no SDK is installed by this script.' }
$sensorOut = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $sensorOut -Force | Out-Null
$sensorExe = Join-Path $sensorOut 'owh-sensors.exe'
& $sensorCompiler /nologo /target:exe /platform:x64 /optimize+ "/out:$sensorExe" (Join-Path $PSScriptRoot 'SensorBridge.cs')
if ($LASTEXITCODE -ne 0) { throw 'Sensor bridge compilation failed.' }
Write-Output $sensorExe
