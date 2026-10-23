# Repository upload status — 2026-10-23

The **v0.3-test7 source and documentation are now on main**, alongside the consolidated root launch CMD scripts.

## Latest source layout

- `src/unified/`: x86 universal architecture selector, PE checks, bundle diagnostics.
- `src/x86/`: x86 test2 Hook and test3 launcher sources.
- `src/x64/`: dual-offset Win7/8.1 Hook source and historical Win7-only Hook source.
- `src/`: original v0.2-test3 x64 source retained as reference.
- Root `00`–`10` CMD scripts call **`LegacyEdgeSync.exe`** (the universal selector), not the old directly invoked x64 launcher.
- `docs/CHANGELOG-v0.3-test7.md`, `docs/TESTING-v0.3-test7.md`, `docs/RELEASE-v0.3-test7.md`, `docs/SHA256SUMS-v0.3-test7.txt`.

## Release status

As last checked, the newest **published** pre-release was `v0.1-test3`; the `v0.3-test7` Release page has **not** been created. The new release note is a draft. The v0.3-test7 universal ZIP requires regression on existing VMs before it should be described as tested.

**Source-only checkout note:** The GitHub main branch does not ship compiled `LegacyEdgeSync.exe` and child binaries. Root CMD scripts require a complete build/runtime package; they will not work from a source-only clone until the binaries are supplied.

Windows 7 x86/x64 and Windows 8.1 x64 successes were achieved in earlier test builds. No Win8.1 x86 result is claimed. Launchers are injection-based and have triggered HEUR/QVM20.1 alerts; these have not been independently confirmed as false positives.

See [README](README.md).
