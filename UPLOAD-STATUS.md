# Repository upload status — 2026-10-24

## Current release

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
- [Current release notes](docs/RELEASE-v0.3-test7.md).
- [Current SHA-256](docs/SHA256SUMS-v0.3-test7.txt).
- [Regression checklist](docs/TESTING-v0.3-test7.md).

**Integrity note:** local candidate ZIP checksum 44a672420b8e36b5c9f4bd8a1c3dad35e0175645e2902a4dc193376e740b7238. The GitHub release asset was verified by name and size; match downloaded asset bytes with this SHA-256 when possible. SHA-256 does not establish that an injection-based program is safe.

**Security note:** 360 HEUR/QVM detection has not been vendor-cleared and must not be described as a confirmed false positive.
