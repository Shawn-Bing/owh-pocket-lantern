@echo off
cd /d "%~dp0"
"%~dp0owh-dashboard.exe" -listen 127.0.0.1:8765
pause
