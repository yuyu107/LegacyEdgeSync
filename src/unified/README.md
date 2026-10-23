# Unified selector source

`unified_launcher.c` builds an x86 Windows console program which runs on Win7 / Win8.1 x86/x64 and delegates to **unchanged** child launchers based on Edge's PE architecture.

Rebuild in an environment with clang + lld-link:

```sh
./build-unified-x86.sh
```

Uses `kernel32-unified.def` as its import library definition; dynamically loads the Windows system's `version.dll` for file version checks. No CRT, third-party packer, obfuscator, or browser patch logic is included in this selector. Existing child launchers/hooks remain required and are separate from this folder.

The `--diagnose` mode reads only file headers and version resources, and checks the selected pair. `--bundle-check` checks both packaged PE pairs even if Edge is not installed. The actual sync modification still occurs **only when you explicitly launch** one of the existing child patch modes.
