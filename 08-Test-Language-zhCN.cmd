@echo off
cd /d "%~dp0"
"%~dp0LegacyEdgeSync.exe" --diagnose --lang=zh-CN
echo.
pause
