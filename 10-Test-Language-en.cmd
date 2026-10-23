@echo off
cd /d "%~dp0"
"%~dp0LegacyEdgeSync.exe" --diagnose --lang=en
echo.
pause
