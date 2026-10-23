#!/bin/sh
# Only rebuild the x86 launcher. The pretested v0.3-test2 hook DLL is intentionally unchanged.
set -eu
cd "$(dirname "$0")"
lld-link /dll /noentry /machine:x86 /def:kernel32-x86.def /out:dummy.dll /implib:kernel32-x86.lib
clang -target i686-pc-windows-msvc -c -std=c11 -O1 -fno-builtin -fno-stack-protector -ffreestanding source/x86_launcher.c -o x86_launcher.obj
lld-link /machine:x86 /entry:mainCRTStartup /subsystem:console,5.01 /nodefaultlib x86_launcher.obj kernel32-x86.lib /out:LegacyEdgeSync32.exe
file LegacyEdgeSync32.exe LegacyEdgeSyncHook32.dll
