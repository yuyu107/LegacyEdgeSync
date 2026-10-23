@echo off
setlocal
cd /d "%~dp0"
"%~dp0Edge109SyncFix.exe" --menu
exit /b %ERRORLEVEL%
