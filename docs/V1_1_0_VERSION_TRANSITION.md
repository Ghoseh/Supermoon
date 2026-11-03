# v1.1.0 version-specific recovery and packaging policy

The owner approved this policy at documentation checkpoint `ab2e65a` and authorized
local final artifact preparation. Public package/recipe version is **1.1.0**.
The exact Phase 6-tested DLLs are retained byte-for-byte; no rebuild or PE metadata
edit occurred. Local artifacts await explicit public upload/release approval.

## Installed private RC stays unchanged

The safest immediate owner transition is **no installed-file or record change**.
The installed `1.0.4-rc1-private` already contains the intended public runtime
bytes. Keep using its original matching RC tool/recipe bundle and recovery record;
its policy remains RC-specific, not a relabeled public record. Preparing public
downloads for new users does not require converting the installed RC.

The v1.1.0 package has its own recipe/policy for fresh public installations.
It never imports, rewrites or silently migrates a private RC record. Likewise,
RC tools cannot manage public v1.1.0 records. Do not mix tool bundles/selectors or
change release/hash fields to bypass refusal. Both DLLs must remain a matching pair.

| Identity | Unchanged value |
| --- | --- |
| winmm.dll, 948,224 bytes | `41241c0b8ba3c0aaad2cbb5501631b0559fef862767e2179113b786ddf6103f3` |
| supermoon_playlist.dll, 6,165,504 bytes | `6c0cf4dc66217dfcf08cf790b61e932c7e8012584455d41f6932dc7ec7eb6fb3` |
| NMS.exe, 89,535,560 bytes; Steam 26385139 | `a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109` |
| Locally generated init.bnk, 36,017 bytes | `65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f` |

## Why version matching matters even with identical DLLs

The unchanged `Installation.load_record` requires an exact manifest/selector/hash,
canonical game path, schema, `record.release == recipe.version`, game identity,
expected after-state and verified prior/new backup bytes. `--record` does not bypass
those gates. Public and RC DLLs are identical, but their recovery policies differ.

Wrong-version status uses the existing `CORRUPT_RECORD_REFUSED` label and includes
a release/identity policy-mismatch message. That message does not establish corrupt
bytes or authorize record editing. Use matching tools. Qualification prose alone
does not alter a record contract if version and identity values stay the same.
This release changes only the public recipe version, not the owner's installed policy.

## Verification

The approved proposal passed 11 owned data-only version-policy fixtures. Final
public preparation passes the installation/recovery suite's 39 cases normally and
with applicable UBSan on its owned process helper; Python is not instrumented.
Seventeen public CLI scenarios pass against disposable copies of the supported
executable/native archive and exact DLLs used as data only. No game/DLL instructions
execute and no mutator targets the real game.

Cases cover clean install/status, same-policy update preserving first prior state,
rollback to absence, resume/disable/reinstall, backup/foreign-file refusal, changed
executable disable, post-bank interruption/resume and public/RC record rejection.
The RC fixture's record remains unchanged and usable with matching RC tools after
public refusal. Final ZIP tool/recipe/DLL bytes match the tested staged inputs.
These setup checks are not new native M2/H1, live listening or endurance tests.

## If the owner later wants an explicit installed-policy conversion

This is a proposed operator-controlled procedure for separate approval, not a
command sequence to execute now. Keep the RC usable under its existing tools until
then. No conversion is required merely to distribute identical runtime bytes.

1. With NMS closed and exclusive access, verify the supported game, all four current
   resources, the entire RC recovery tree and its exact matching tool/recipe bundle.
   Keep verified durable copies outside game/TEMP, including a byte-identical snapshot
   of the original record before any tool changes its mutable journal. Preserve
   personal INI, external Music, Cache, State, diagnostics and unrelated files.
2. Use only matching RC tools to inspect and deliberately disable recognized resources.
   The immutable RC manifest stays unchanged; a normal disable updates its mutable
   journal. Verify all four targets absent. On unexpected bytes/failure, stop and
   report state; do not run the new tool or attempt unsafe rollback.
3. Preserve/read-back-verify the complete disabled RC recovery tree, then archive
   that tree outside the game using the reviewed manual procedure. Retain both
   original and disabled snapshots and matching tools. Archived records remain
   bound to the original canonical game path; they are not portable policies to
   run in an arbitrary new location. Do not merge old/new recovery roots/selectors.
4. Only with a verified fresh recovery root, install the approved public package
   under its matching v1.1.0 policy. It creates a separate all-absent prior record.
   Verify installed bytes, record/selector/journal/backups and preservation of
   personal/unrelated data. No source music/configuration restoration is needed.
5. Public rollback/disable returns absence, not the private RC or v1.0.3. Restoring
   an old setup requires a separate supported-game/whole-pair/bank check and matching
   trusted record/tool procedure; never replace newer State/preferences with old
   personal backups or force an unsupported game. Keep the RC snapshots intact.

Before authorizing that real conversion, exercise the chosen exact public package
and the archive/restore stages in disposable fixtures, including interruptions,
foreign files, corrupt backups, changed executable and personal-data preservation.
Current work does not test or perform that future operator conversion end-to-end.

## Final local artifacts and provenance

`Supermoon-1.1.0-windows-x64.zip` and `Supermoon-1.1.0-source.zip` are assembled in
a new isolated output directory, independently twice, with identical bytes. They
never overwrite the sealed private candidate or v1.0.3. The external release receipt
and release `SHA256SUMS` provide final ZIP hashes and manifest pins; embedded metadata
does not attempt a circular hash of its own containing ZIP.

Production source remains checkpoint `50a8577`; the approved documentation basis
is `ab2e65a`. The source archive includes updated documentation, public recipe,
packaging-member selection and manifest metadata with unchanged production/test
inputs. The final preparation checkpoint is identified by the external receipt.

The installation contains validated README/INSTALL/changelog, release notes,
offline Nexus description, this guide, qualification/limitations and required
licenses/asset notices. Only example INI ships. Native/generated banks, game files,
private records/evidence, music, personal configuration, Cache/State/logs and
credentials are excluded. Bank generation remains local and exactly pinned.

Source/member manifests and checksums are refreshed deliberately. The original
Phase 5 packaging summary is preserved as `V1_0_4_RC1_PRIVATE_PACKAGING_VERIFICATION.json`;
`PACKAGING_VERIFICATION.json` separates retained runtime evidence from new checks.
Archive inventories, CRC/path/case/hash/checksum/source correspondence, links,
privacy selection and notices are verified before owner publication review.

## Stop boundary

M2/H1 acceptance is already recorded for the unchanged limited desktop scope.
Neither issue is fixed or a passing regression. Public upload, remote release
creation, tags, installed migration and game launch/attach require separate
authorization. Local artifact preparation does not grant any of those permissions.
