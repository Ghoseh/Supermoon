# Build and provenance

Supermoon **1.0.2 candidate** targets Windows x64 and NMS Steam build **26295378** only.
The supported `NMS.exe` is 89,489,480 bytes, SHA-256:

`d393c6782d29e1915854c0d5cf42505c8093740c05dc53705dbed7e0d1d4ac37`

Startup verifies the executable, loaded native sites and payload separately.
A matching general game version label is insufficient. Do not change identity
pins to force an unknown update to activate.

## Toolchain

Use Python 3.10+ and Zig 0.15.2 for Windows x86_64. Official archive:
<https://ziglang.org/download/0.15.2/zig-x86_64-windows-0.15.2.zip>.

- Archive SHA-256: `3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c`
- `zig.exe` SHA-256: `d408dd38eed3e5204af841bcebf70502a4dbbb8399a3a3262be55059370bc018`
- Driver: Clang 20.1.2; target `x86_64-unknown-windows-gnu`.

The toolchain is not bundled. No game executable, SDK or private native fixture
is required to compile the mod.

## Build

From this source checkout, run:

```powershell
python -B tools/build.py --zig <zig.exe> --dry-run
python -B tools/build.py --zig <zig.exe>
```

The wrapper creates a fresh disposable directory outside the checkout and isolates
compiler caches and temporary paths. It builds the pinned modified
`ebur128_peak.c`, nineteen payload units and four proxy units. `build/recipe.json`
records the unit list and expected binary identities. The C++ recipe uses C++17,
Microsoft extensions, `ADAPTER_RECOVERY_BUILD`, O2 and preserved warning flags;
the meter uses gnu99. Do not substitute unmodified vendor `ebur128.c` or omit
`recovery_native.cpp`.

Post-link normalization clears informational CodeView records and COFF/export
timestamps while preserving runtime PE directories. The wrapper checks the existing
payload pin and both final hashes; it does not rewrite source pins or install its
output. Keep private debug files and command receipts out of distributions.

The candidate pair was built and reproduced with the portable wrapper in a
second disposable directory. Both builds matched RELEASE_DLL_SHA256SUMS and
build/recipe.json. The pair is staged separately and has not been installed.

## Verification scope

The native profile was adapted from the published v1.0.1 integration against
the new owned executable, preserving the v1.0.0 behavior reference. Independent
relocation, unwind, bridge, allocation/lifecycle and UI reviews passed, together
with native-fixture and public regressions. Live acceptance is pending.
See [qualification](QUALIFICATION.md) and [migration evidence](MIGRATION_STATUS.md).

`PUBLIC_FILE_MANIFEST.json` and `SHA256SUMS` describe the selected source package.
The installation ZIP has its own manifest and checksums. No music, caches, state,
logs, saves, game files or private fixtures are included. `.gitattributes`
preserves source bytes and marks embedded asset include files as generated for
GitHub language statistics.
