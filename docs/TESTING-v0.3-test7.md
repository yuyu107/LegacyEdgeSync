# v0.3-test7 release checklist (not a stable release)

## Static checks

- [x] x86 Hook DLL binary equals the successful v0.3-test2/v0.3-test3 baseline.
- [x] x86 child launcher equals the previous successful v0.3-test3 build.
- [x] x64 child launcher equals the previous v0.2-test3 build.
- [x] x64 Hook DLL binary equals the successful Win8.1 v0.3-test6 test binary.
- [x] Original Win7 x64-only Hook retained for manual rollback.
- [x] Universal x86 selector compiles on clang/lld-link and imports only Windows 7 APIs.
- [x] ZIP and SHA256SUMS contents validated.

## Runtime regression — use existing VMs; do not install Win8.1 x86 solely to test this

- [ ] Win7 x86: `--bundle-check` and `--diagnose`, then independent-profile sync.
- [ ] Win7 x64: `--bundle-check` and `--diagnose`, then independent-profile sync (`patch_at_0x49>0`).
- [ ] Win8.1 x64: `--bundle-check` and `--diagnose`, then independent-profile sync (`patch_at_0x45>0`).
- [ ] Existing-profile mode only after full browser exit and local backup.
- [ ] Windows restart: profile remains available, sync restarts without repeated login.
- [ ] Test-account bookmark add/rename/delete between two devices. Do not test real credentials.
- [ ] Three language modes and command-line forwarding verified on each OS used.
- [ ] Error cases: missing DLL, wrong architecture, version mismatch, browser already running.
- [ ] Antivirus false-positive evaluation/disclosure reviewed; do not disable protection to force tests.

## Deferred

- [ ] Win8.1 x86 — deferred until someone already has that environment.
- [ ] Win7 x64 + Edge x86 — optional, no new VM necessary.
- [ ] Profile avatar research.
