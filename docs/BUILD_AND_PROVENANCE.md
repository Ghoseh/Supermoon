# Source, dependencies and reproducible build

Public policy/version:1.2.0. Runtime source basis:5d46716 (packaging basis0acfd0b).
The complete source/ tree, ebur128_peak.c, winmm.def and native build scripts are
unchanged from the frozen live-tested pair. Public packaging/qualification prose
and recovery-policy metadata do not alter those bytes. Runtime PE labels can retain
the private build's metadata; this is intentional preservation, not a mismatched pair.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| winmm.dll | 948224 | `e2ff82fec494f62549a9de6e94fee90a07292170d57ec1b94e8e2cbf94533b56` |
| supermoon_playlist.dll | 6165504 | `0bb13ce24c6adaf351245ce95fcecfd4373e43c0ef9b17531ced23e15543d2fe` |
| NMS.exe | 89540680 | `092236962ee050e419424699b5c343a0173f7773d5ad0866b7ec366d334c07d4` |
| Native NMSARC.audioBNK.pak | 67842512 | `29682a3015f0ffcfa1ac236e4d49a9670e6d2a79e50a25315f8faecf45cde940` |
| Generated init.bnk (not distributed) | 36017 | `65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f` |

## Developer requirements (not end-user requirements)

Use Windows x64, Python 3.10+ standard library, and the Windows x64 Zig 0.15.2
toolchain archive (Clang 20.1.2, target x86_64-unknown-windows-gnu). No pip packages
are needed for the build. No NMS SDK/headers, game binary or private music is part
of the source/build inputs. Windows supplies its UCRT and system APIs.
Zig archive SHA-256:3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c.
zig.exe SHA-256:d408dd38eed3e5204af841bcebf70502a4dbbb8399a3a3262be55059370bc018.
Obtain the matching archive from the official Zig download archive, verify the
trusted hash and extract the whole toolchain together. It is not redistributed.

From the extracted source root, using an explicit path to that zig.exe:

```powershell
python -B tools/verify.py
python -B tools/build.py --zig 'C:\Tools\zig-0.15.2\zig.exe' --dry-run
python -B tools/build.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
```

build.py copies source into fresh isolated temporary output, uses the pinned
recipe's19 payload/four proxy units and generated meter, normalizes PE metadata,
and requires both exact hashes. It never installs, updates pins or launches NMS.
Wrong compiler/payload refuses. Build outputs go outside source; copy only verified
DLLs when deliberately packaging. This release copied the frozen pair and did not
rebuild it; prior independent review compared two retained reproducible outputs.
Final preparation verifies byte-for-byte input correspondence and dry-run commands.

## Packaging and tests

```powershell
python -B tools/package_installation.py --dll-dir 'C:\VerifiedDLLs' --output 'C:\Output\Supermoon-1.2.0-windows-x64.zip'
python -B tools/package_source.py --output 'C:\Output\Supermoon-1.2.0-source.zip'
python -B tools/verify.py 'C:\Output\Supermoon-1.2.0-windows-x64.zip'
python -B tools/verify.py 'C:\Output\Supermoon-1.2.0-source.zip'
```

Outputs require new names and never overwrite releases. The source builder selects
only PUBLIC_FILE_MANIFEST.json's pinned portable files. It does not discover or
refresh private files. SHA256SUMS covers files and manifest; final ZIP hashes are
external to avoid circularity. Compare against a separately trusted receipt.
See [Developer guide](DEVELOPER_GUIDE.md) and [Tests](../tests/README.md).
Licenses/notices accompany source and binaries; author attributions are unchanged.
