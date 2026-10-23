@echo off
cd /d "%~dp0"
"%~dp0LegacyEdgeSync.exe" --bundle-check
echo.
pause
