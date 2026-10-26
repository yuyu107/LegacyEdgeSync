# Repository upload status — 2026-10-26

## v1.0 video release published

- Chosen public version: **v1.0**, based on the v0.3-test8 adaptive test ZIP.
- **Published on 2026-10-26** as the [v1.0 GitHub Release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v1.0).
- Verified asset metadata: `LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip` (125174 bytes); uploaded asset bytes have not independently been downloaded and SHA-256-verified.
- Five principal EXE/DLL binaries are **byte-identical to v0.3-test8**. Internal historical component build labels intentionally remain visible.
- New adaptive x64 source exists in the v1.0 source kit, but **the repository's current src/x64/hook.c still reflects v0.3-test7**. Do not claim the GitHub main branch already contains the v1.0 adaptive implementation.
- Local pre-upload ZIP SHA-256: `a4994f4315088057f32b79ae88b31eb7972713b43e0564db695eddda9dde303e`.
- v1.0 files: `docs/RELEASE-v1.0.md`, `docs/CHANGELOG-v1.0.md`, `docs/TESTING-v1.0.md`.

## Historical published releases

- **v0.3-test7 is published** as a [GitHub pre-release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7).
- Verified release asset entry: `LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip` (114435 bytes).
- Published release does **not** imply full Win7/8.1 runtime regression. Successful per-architecture tests were from earlier experimental builds.
- Previous pre-release: [v0.1-test3](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3).

## Source layout

- `src/unified/` — architecture selector, bundle diagnostics, build instructions.
- `src/x86/` — x86 child launcher and three-path Hook sources.
- `src/x64/` — Win7/Win8.1 dual-offset Hook and prior Win7-only Hook source.
- Root `src/` files — historical v0.2-test3 x64 implementation.
- Root numbered CMD scripts — universal runtime shortcuts; **source-only GitHub checkout does not contain their required EXE/DLL binaries**.
- `legacy/v0.1-test3/` — archived legacy CMD helpers, no longer competing with new root numbers.

## Documentation

- [README](README.md) — current download and instructions.
- [Release index](docs/RELEASES.md) — published versions and archived test milestones.
- [Current release notes](docs/RELEASE-v1.0.md).
- [Current SHA-256](docs/SHA256SUMS-v1.0.txt).
- [Regression checklist](docs/TESTING-v1.0.md).

**Integrity note:** local v1.0 ZIP checksum a4994f4315088057f32b79ae88b31eb7972713b43e0564db695eddda9dde303e. The GitHub v1.0 asset was verified by name and size, not independently re-downloaded and hashed. SHA-256 does not establish that an injection-based program is safe.

**Security note:** 360 HEUR/QVM detection has not been vendor-cleared and must not be described as a confirmed false positive.
