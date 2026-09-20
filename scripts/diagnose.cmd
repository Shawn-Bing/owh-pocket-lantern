@echo off
cd /d "%~dp0"
start "" /wait "%~dp0overwatch-helper.exe" --diagnose
if errorlevel 1 (
  echo Diagnosis failed. Check configuration directory permissions.
  pause
  exit /b 1
)
start "" notepad.exe "%LOCALAPPDATA%\OverwatchHelper\diagnostics.txt"
