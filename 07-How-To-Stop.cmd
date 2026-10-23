@echo off
setlocal
cd /d "%~dp0"
"%~dp0Edge109SyncFix.exe" --guide
set "R=%ERRORLEVEL%"
pause
exit /b %R%
