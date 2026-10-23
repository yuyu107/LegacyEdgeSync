@echo off
echo To stop the in-process hook, close the TEST Edge profile and make sure all its msedge.exe processes have exited.
echo The original on-disk Microsoft Edge files have not been modified.
echo No Fiddler certificate or Windows proxy was installed.
echo The test profile remains in %%TEMP%%\Edge109SyncFix-Test for manual deletion after backups.
echo DO NOT kill all Edge processes if other profiles have unsaved work.
pause
