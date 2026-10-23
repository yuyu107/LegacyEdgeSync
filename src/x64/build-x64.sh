#!/bin/sh
# Rebuild the diagnostic x64 DLL. Requires clang + lld-link. No C runtime.
set -eu
cd "$(dirname "$0")"
lld-link /dll /noentry /machine:x64 /def:kernel32.def /out:kernel32-imports.dll /implib:kernel32-imports.lib
clang -target x86_64-pc-windows-msvc -c -std=c11 -O1 -fno-builtin -fno-stack-protector -ffreestanding hook.c -o hook_test6.obj
lld-link /dll /machine:x64 /entry:DllMain /subsystem:windows,6.01 /nodefaultlib hook_test6.obj kernel32-imports.lib /out:../Edge109SyncHook.dll
