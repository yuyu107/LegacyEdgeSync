@echo off
cd /d "%~dp0"
"%~dp0LegacyEdgeSync.exe" --diagnose --lang=zh-TW
echo.
pause
