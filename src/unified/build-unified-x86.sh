#!/bin/sh
# Build Win7 x86 unified dispatcher with clang/lld-link; no C runtime.
# Does NOT rebuild or touch either Hook DLL or existing child launchers.
set -eu
cd "$(dirname "$0")"
lld-link /dll /noentry /machine:x86 /def:kernel32-unified.def /out:unified-import-dummy.dll /implib:kernel32-unified.lib
clang -target i686-pc-windows-msvc -c -std=c11 -O1 -fno-builtin -fno-stack-protector -ffreestanding unified_launcher.c -o unified_launcher.obj
lld-link /machine:x86 /entry:mainCRTStartup /subsystem:console,5.01 /nodefaultlib unified_launcher.obj kernel32-unified.lib /out:../LegacyEdgeSync.exe
