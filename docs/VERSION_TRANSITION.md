# v1.2.0 version-specific recovery

Public policy 1.2.0 uses the exact live-tested26540391 pair. It is distinct from
private 1.1.1-nms26540391-pythonfree-private, public 1.1.0 and RC 1.0.4-rc1-private.
Schema compatibility and equal DLL hashes do not make records interchangeable.
No automatic import/relabel/merge exists. Keep matching tools/records together.

The already installed live-tested private candidate can remain unchanged; preparing
or publishing1.2.0 does not require converting it. Public tools must refuse its
private record. To migrate later, use matching private PowerShell Disable, verify
all four resources absent, preserve original/disabled records and archive the old
root outside game/TEMP, then Install with public tools into a fresh recovery root.
Verify new prior state all absent and exact pair/bank/marker after install.

Old public 1.1.0/RC DLLs target 26385139 and must never reactivate on 26540391. Their
matching legacy Disable may require Python and can remove recognized resources
after an executable update; old Install/Resume/Rollback must refuse. Do not treat
retained old DLL backups as a fallback. Missing matching tools/ownership evidence
requires review, not record edits or wholesale folder deletion.

If migration/public installation fails, keep NMS closed, preserve all evidence and
choose matching Resume or all-absent-prior Rollback/Disable deliberately. Safe
fallback requires BOTH DLLs, marker AND generated bank absent, with personal files
and records retained. Failed rollback may leave the bank after DLL removal; do
not launch until all four are absent. Resolve identified sharing errors and retry
explicitly; corrupt/unknown state requires ownership-qualified recovery review.
Never restore personal save/settings/Cache/State from historical snapshots merely
to satisfy a test hash. Normal gameplay changes are valid. See INSTALL.md and
NMS26540391_MIGRATION.md for exact resource paths and recovery detail.
