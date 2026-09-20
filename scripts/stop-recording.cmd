@echo off
cd /d "%~dp0"
start "" /wait "%~dp0owh-recorder.exe" --stop
if errorlevel 1 (
  echo Recorder was not found. Open the recorder window to check its status.
) else (
  echo Stop requested. Wait for the recorder to finish saving.
)
pause
