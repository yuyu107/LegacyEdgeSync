@echo off
start "" "%ProgramFiles(x86)%\Microsoft\Edge\Application\msedge.exe" --user-data-dir="%TEMP%\Edge109SyncFix-Test" --sync-url="https://edge.microsoft.com/sync/v1/feeds/me/syncEntities" edge://sync-internals/
