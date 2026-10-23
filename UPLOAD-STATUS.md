# Repository upload status — 2026-10-23

The source repository now contains the **v0.2-test3 multilingual launcher code** and the original runtime hook implementation. GitHub Desktop has successfully synchronized the main source files.

## Verified repository files (main branch)

- `src/launcher.c` — v0.2-test3 multilingual launcher, Git blob SHA `ab8ccef668a5117d70a3922ce377ad82122cd63b`
- `src/winmini.h` — updated Windows 7 API declarations, SHA `ee127b517fc60bcaeecd4cc78c4a36b2f4a0ac90`
- `src/hook.c` — retained tested hook source, SHA `fbb6b127abba0c2402b6213ea3fd1152b9346dbe`
- `src/kernel32.def`, startup CMD wrappers and project documentation are present.
- `docs/WHY-SYNC-FIX.md` and `docs/WHY-NOT-EXTENDED-KERNEL.md` are published.

## Important release distinction

The currently published GitHub Release is **v0.1-test3 (pre-release)**. Its asset is `Edge109-SyncFix-v0.1-test3-Win7-x64.zip`.

The **v0.2-test3 source is uploaded**, but the corresponding compiled multilingual binary has **not** been published as a GitHub Release. Do not assume the v0.1-test3 binary has the current multilingual menu or existing-profile mode.

See [README](README.md) for current usage, compatibility and limitations.

This is an experimental prototype, not a security update or a supported modern-browser replacement.
