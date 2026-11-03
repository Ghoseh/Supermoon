# Build and provenance

Supermoon **v1.1.0** retains the exact DLL pair qualified as **1.0.4-rc1-private**
in Phase 6. The public recipe/recovery version is `1.1.0`; the installed RC remains
on its original record/tools. No DLL rebuild or PE metadata edit occurred.
Scope is Windows x64 ordinary desktop NMS Steam build **26385139** only.

The supported `NMS.exe` is 89,535,560 bytes, SHA-256:

`a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`

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

The tested pair retains its deliberate Phase 5 identity pins. The Phase 5
private report records seed linking, pinned-wrapper reproduction in independent
output directories, individual production-unit compilation and archive verification.
The wrapper uses the existing expected identities and never silently repins them.
No game/native-site/bank pin was relaxed. Reproducible bytes do not imply runtime
qualification. Phase 6 separately supplies finite live evidence. M2 and H1/U1
remain unresolved with unchanged failed tests/severities and historical audit
classifications; the owner explicitly accepted their risks for the limited scope.

## Verification scope

Phase 6 verified installation and the owner-run desktop checklist in two sessions,
including normal Stop restoration and normal exits. SDK submission and listening
remain separate evidence. Direct winning-bank identity, all native schedules,
representative long-session headroom and a new four-hour endurance remain unqualified.
Historical v1.0.3 hashes differ from this pair. See [qualification](QUALIFICATION.md),
[Phase 5](REMEDIATION_PHASE5.md) and [migration status](MIGRATION_STATUS.md).

The required audio override is generated from the user's supported native bank
using tools/prepare_audio.py and tools/audio_patch.json. Python 3.10+ needs no
additional packages. The persistent staging package retains those two files under
Supermoon/tools and the recipe under Supermoon/build. The native archive is not
modified; neither native nor generated bank is distributed. See INSTALL.md.

`PUBLIC_FILE_MANIFEST.json` and `SHA256SUMS` describe the deliberately selected
v1.1.0 source closure. Recipe qualification reflects Phase 6 and owner-accepted
risks; `PACKAGING_VERIFICATION.json` distinguishes retained runtime tests from new
public setup/package checks. The original Phase 5 summary is retained separately
as `V1_0_4_RC1_PRIVATE_PACKAGING_VERIFICATION.json`. No identity gate is relaxed.
The installation ZIP has its own manifest and checksums. No music, caches, state,
logs, saves, game files or private fixtures are included. `.gitattributes`
preserves source bytes and marks embedded asset include files as generated for
GitHub language statistics.

## Reproducible archive assembly

The owner approved local v1.1.0 artifact preparation and the separate recovery
policy. Packaging uses the retained DLLs, not a rebuild. See the
[version policy](V1_1_0_VERSION_TRANSITION.md). This does not authorize uploading,
creating a remote release or changing the owner's installed RC.

After checking DLLs against the recipe, `tools/package_installation.py --dll-dir
<build-output> --output <fresh-install.zip>` creates and verifies the deterministic
installation ZIP without a live INI. `tools/package_source.py --output <fresh-source.zip>`
selects the manifest-pinned source closure, verifies its bytes and makes
a deterministic source ZIP. Both use exclusive output creation. Verify either ZIP
with `python -B tools/verify.py <zip>`. Retain the external trusted archive/manifest
hashes; a self-consistent archive is not a publisher signature.
