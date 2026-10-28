> This branch targets Steam build 26271400. The repair build and offline tests ran on 2026-10-28; see MIGRATION_STATUS.md. Historical packaging notes below describe the original F6 work.

# Portable build and provenance

## Supported inputs

Windows x64; Python 3.10+; Zig 0.15.2 x86_64-windows distribution. Download the
official archive from https://ziglang.org/download/0.15.2/zig-x86_64-windows-0.15.2.zip
and verify SHA-256 before extracting:

`3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c`

Expected zig.exe SHA-256:
`d408dd38eed3e5204af841bcebf70502a4dbbb8399a3a3262be55059370bc018`.
The actual cc driver reports Clang 20.1.2; target x86_64-unknown-windows-gnu.
The toolchain is not bundled; keep its own complete licenses when redistributing it.

The only supported NMS.exe profile in this release is 89,491,528 bytes, SHA-256:
`d1c7155764271eff0733c37d1849e685a11da3d54a41d685d7898fc0aad83a87`.
A numeric game release label was not independently established. Startup verifies
the executable and native sites; do not change pins to force a game update to load.
No game SDK, game executable or additional downloaded library is required to compile.

## Build from a byte-preserving checkout

Run from this repository (replace the tool path with your own):

```powershell
python -B tools/build.py --zig <zig.exe> --dry-run
python -B tools/build.py --zig <zig.exe>
```

The build creates and prints a fresh disposable directory outside the source tree.
It copies source there, isolates compiler/temp caches, compiles the pinned modified
ebur128_peak.c, then the payload and proxy. Nineteen payload and four proxy units
use C++17, Microsoft extensions, ADAPTER_RECOVERY_BUILD, O2, g0 and the exact warning
flags in the preserved recipe; the meter uses gnu99. build/recipe.json lists units.
Do not compile pristine vendor ebur128.c in place of ebur128_peak.c or omit recovery_native.cpp.
The exact generated winmm.def and bootstrap payload hash pin are included.

Post-link normalization zeros informational CodeView records and COFF/export
timestamps while preserving exports, imports, exception and relocation directories.
It reproduces the qualified normalization; it is not a runtime-code transformation.
The wrapper verifies the payload against its existing bootstrap pin and both final
hashes, refusing an unexpected result rather than silently rewriting source pins.
No output is installed or copied over a release. Keep private compiler PDBs and
command receipts out of public releases even if a local linker creates them.

The original F6 pair was previously reproduced in separate native builds and independently
verified. This public packaging pass does not rebuild it: the portable wrapper's
commands, input closure and dry-run are checked, with compiler preprocessing of all
24 production units to verify copyright-comment equivalence. A fresh full build
using the public wrapper has not been executed during this packaging-only task.
Its hash checks are the acceptance criteria when reproducing locally. See
PACKAGING_VERIFICATION.json and QUALIFICATION.md for evidence boundaries.

Source/ is the complete retained production/support source set. Vendor/provenance,
generated profiles, assets, example configuration, meter C and DEF remain exact RC1
inputs. First-party copyright additions are comments only. Public docs/build/test
wrappers are selected separately from historical scripts with private output paths.
Git attributes disable automatic line-ending conversion to preserve these inputs.
SHA256SUMS and PUBLIC_FILE_MANIFEST.json verify public bytes; RELEASE_DLL_SHA256SUMS
pins the separate installation ZIP's exact runtime pair.
