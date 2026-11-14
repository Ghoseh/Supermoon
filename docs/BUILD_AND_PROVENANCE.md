# Source and build provenance: Supermoon 1.2.1

Base public source commit: a8d3c652edd98979aea1c9693a4e8867082d96f2.
This release was rebuilt from the included source with the existing Zig 0.15.2
toolchain/flags and PE normalization. It is not byte-identical to v1.2.0, the
live gain precursor or the combined private candidate. The final binary includes
the +3 missing-setting default and fail-neutral unreadable-INI checks. Native
callback/connection writes, PCM/normalization/cache and playback lifecycle are
unchanged from the qualified precursor; keyboard modifier parsing is retained.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| winmm.dll | 948224 | `2fccc5d3bbdb700a5b6f5acf72eb739d99b321f889013d413ef3e630c388efc4` |
| supermoon_playlist.dll | 6341632 | `50511f0ddf93be6da9d09ecb340fcb16961d76bc1f9e74f0635733fee472fbae` |
| NMS.exe | 89540680 | `092236962ee050e419424699b5c343a0173f7773d5ad0866b7ec366d334c07d4` |

Developer requirements: Windows x64, Python 3.10+ standard library and the pinned
Windows x64 Zig 0.15.2 toolchain (Clang 20.1.2). No game binary, native byte fixture
or music is needed to build. Windows provides its UCRT/system APIs. Zig is not
redistributed. Archive SHA-256:
`3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c`.
zig.exe SHA-256:
`d408dd38eed3e5204af841bcebf70502a4dbbb8399a3a3262be55059370bc018`.

```powershell
python -B tools/verify.py
python -B tools/build.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
python -B tools/run_tests.py --zig 'C:\Tools\zig-0.15.2\zig.exe' --suite personal_gain_tests --suite keyboard_tests --ubsan
```

build.py reserves fresh output outside the source, uses 19 payload/four proxy
production units, normalizes PE metadata and requires both exact pinned hashes.
It does not update pins, install, load the game or publish. End-user installation
uses Windows PowerShell instead of these developer tools.

```powershell
python -B tools/package_installation.py --dll-dir 'C:\VerifiedDLLs' --output 'C:\Output\Supermoon-1.2.1-windows-x64.zip'
python -B tools/package_source.py --output 'C:\Output\Supermoon-1.2.1-source.zip'
```

Source packaging verifies the selected PUBLIC_FILE_MANIFEST and SHA256SUMS.
Archives use fixed ZIP metadata and exclusive output creation. License notices,
Malik Seals attribution and dependency provenance remain included. The Windows
ZIP omits development tools/tests/internal qualification material. Matching
public v1.2.0 recovery reader/Disable code is bundled verbatim for Upgrade; old
DLLs and game data are not included. See [qualification](QUALIFICATION.md).

INI distribution revision: complete active/template INI, additive missing-key
completion with original backups and atomic writes, idempotent retry, retained
user configuration on Disable/rollback. These installer-only changes do not
change either qualified DLL. Separate INI and extracted-package tests apply.
