# Changes: v0.3-test7 (Windows 7 / 8.1 consolidation)

- Bundled the **binary-identical** x64 Hook DLL from the successful v0.3-test6 Win8.1 experiment (SHA-256 in `SHA256SUMS.txt`).
- Retained the original exact `base+0x49` path for Win7 x64, plus exact `base+0x45` fallback demonstrated working on Win8.1 x64.
- Bundled the **unchanged** x86 Hook DLL from the successful v0.3-test2/v0.3-test3 test build. No new x86 Hook code was introduced.
- Rebuilt only the **universal launcher** with an additional `--bundle-check` mode: validate both child EXEs and DLLs by PE architecture, without Edge installation or injection.
- `--diagnose` now confirms the matching child EXE and Hook DLL, not merely Edge PE machine/version.
- All prior commands and language handling remain available; old x86/x64 injection executables were **not recompiled**.
- Included the old Win7 x64 Hook DLL and its source in `Rollback-Hooks/` for manual comparison / rollback.
- Windows 8.1 x86 is **not tested** and there is no need to build another VM solely for this pre-release.
- Updated safety software disclosure and regression plan; not an antivirus evasion effort.

## Validation provenance

- Win7 x86: previous tested x86 Hook from v0.3-test2; x86 normal profile mode also reported successful in v0.3-test3.
- Win7 x64: previous tested v0.2-test3 Hook `base+0x49`; dual-offset DLL still requires Win7 regression.
- Win8.1 x64: v0.3-test6 had `hits=14`, `patched=14`, `patch_at_0x45=14`, `SYNCER_OK`.
- v0.3-test7 rebuilt universal launcher and complete bundle: **static PE/build/hash checks only; runtime not independently tested**.
