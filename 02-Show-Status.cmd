@echo off
if not exist "%TEMP%\Edge109SyncFix-status.txt" (
  echo No status file. Launcher may not have injected successfully.
) else (
  type "%TEMP%\Edge109SyncFix-status.txt"
)
pause
