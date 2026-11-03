> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Phase 3: installation, updates and recovery

2026-11-02. Implemented in the existing isolated remediation worktree after
verifying the Phase 2B evidence seal. This is an offline source checkpoint, not
a new qualified runtime pair or published release. M2/H1 remain unresolved.

## Changes

- `tools/package_installation.py` assembles a private candidate from the exact
  recipe-pinned DLLs, with verified manifest/checksums, complete existing notices,
  setup tools and `playlist.example.ini`. No live INI, bank, music, cache or state
  is packaged. Existing output archives are never overwritten. The original sealed
  v1.0.3 archive is unchanged and still requires the old-package warning in INSTALL.md.
- `tools/install_supermoon.py` manages only the proxy, payload, marker and required
  bank. Package extraction is into separate staging; the installer copies only
  those program resources and preserves personal/unrelated files by construction.
  Exact executable, DLL, marker, native archive, inner-bank and derived-bank gates
  remain mandatory. Unknown current program bytes are refused.
- Before replacement, prior and candidate bytes are stored under the persistent
  game's `.Supermoon-Recovery/records/<id>/`. Files are flushed/read back; the
  immutable manifest binds game path, release, game/DLL/bank identities and prior
  absence/presence. A hash-checked active selector follows verified backups. Retries
  preserve the first prior state; completed rollback/disable permits a new record
  without deleting old records.
- Status rehashes actual files independently of the mutable journal. Resume uses
  saved candidates without needing the extracted package or native archive.
  Rollback requires a qualified matching prior state, and disable refuses unknown
  files. Unsupported game updates allow disable, not forced restoration/activation.
  A legacy menu-only bank can be backed up but is not qualified for automatic rollback.
- The activation marker is removed first and restored last. Each target and each
  saved replacement is revalidated before use. Failures after bank replacement
  explicitly report incomplete work and the recovery location. Corrupt records,
  unknown current files and unqualified prior combinations are not overwritten.
- `tools/installation_safety.py` uses Windows Toolhelp process enumeration without
  launching `tasklist` or searching for executables. Incomplete/error/empty snapshots
  and the entry bound fail closed. An exclusive Windows handle serializes owned
  writers; process death releases it. Reparse ancestry and hard-linked managed
  files are refused. The lock and process snapshot do not coordinate Steam/NMS startup.
- `prepare_audio.py --install` delegates to the durable mechanism for an already
  matching DLL pair/marker. Its generation/patch algorithm and numeric bank plan
  are unchanged. This compatibility route cannot recover prior DLLs overwritten
  by an earlier manual extraction; the full installer is the supported update path.

See [INSTALL.md](../INSTALL.md) for commands and conservative manual handling of
unknown/corrupted files. Tools recognize this exact trusted recipe, not arbitrary
old/new versions. Keep the trusted matching tools and records for recovery.

## Supported filesystem contract and limits

One writer, NMS closed, Steam updates and other file writers idle, persistent local
game/recovery directories. Recovery storage under the active TEMP root is refused.
Cooperative locking does not establish safety against arbitrary concurrent file or
directory replacement. A retained fixture demonstrates that a noncooperating
mutation after the final check can still be overwritten; this is explicitly outside
the advertised contract. Pre-existing and post-preparation junction checks refuse
the tested path changes with outside sentinels intact.

The four files are not an atomic whole-install transaction. Complete new files can
coexist with incomplete journal/other-file progress. Status exposes both. Flush,
readback, atomic individual replacement and process-kill tests do not establish
power-loss/directory-metadata durability. Backups are durable relative to TEMP
cleanup; copy them outside the game before deleting or relocating the game.
Local records are integrity checked, not a signature or defense against malicious
modification of both trusted tools and records. No force option or automatic
unknown-file/scratch cleanup is supplied. No universal version migration,
mod management, live unload or shutdown guarantee is introduced.

## Checks performed

- Installation fixtures: **39 tests PASS normally and 39 PASS with the owned
  process helper compiled under UBSan**. Python code is not UBSan-instrumented.
  Includes clean/default and personal-INI installs; accepted/current/unknown banks;
  missing/corrupt program inputs; record/backup/journal corruption; qualified and
  refused rollback; unsupported-game disable; repeated retries; TEMP cleanup;
  backup/scratch/receipt/replace/post-commit failures; real Windows sharing and
  writer exclusion; junction/path changes; native process errors and a benign
  cwd/PATH `tasklist.exe` sentinel that never executes.
- Five exact-child termination stages cover prepared, before/after bank replacement,
  before final verification receipt, and after removing an existing activation marker.
  Resume retains the original manifest and rollback restores its prior state.
  Only owned synthetic fixture processes are killed/reaped. The process named
  NMS.exe in the guard fixture contains only the newly compiled test helper.
- **13 CLI checks PASS** against private copies of the exact pinned executable,
  archive and baseline DLLs. Inputs are data only: no NMS or DLL instructions run.
  Covers verified example-only ZIP, read-only status, real bank derivation, clean
  install/retry, bank-only compatibility, resume after staged-package deletion,
  rollback/reinstall/disable and corrupted-package/foreign-proxy refusal.
- Complete public suites PASS normally and under UBSan: eleven suites plus the
  **595-check** parser corpus, **25 successful command receipts per run**. The
  normal run includes the qualified proxy's wrong-host forwarding check. Existing
  cache tests verify absent-INI runtime defaults and optional configuration.
- Complete unchanged remediation suites return failure honestly in both modes:
  **81 PASS, three M2 FAIL, one H1 TIMEOUT, zero ERROR**. No sanitizer diagnostics
  were observed. The same three final-release failures and original H1 self-wait
  remain; no assertion/condition was removed, weakened, bypassed or relabeled green.
- All runtime sources/assembly/pins, the audio patch plan, build recipe, and original
  C++ regression files remain byte-identical to the starting checkpoint. The sealed
  release, installed identities, original audit evidence and prior phase seals
  remain unchanged. No installed setting/file was changed, no game was launched,
  and no release was published or runtime DLL rebuilt.

Evidence and full receipts:
`private evidence: PHASE3_REPORT.txt`.
Phase 2B remains sealed at manifest SHA-256
`16cf428a52658f7e375f2226ed0d1fad476aa4355f523bcbd36018e7fcf9c7e7`.
Stop after this checkpoint for owner review before further remediation.
