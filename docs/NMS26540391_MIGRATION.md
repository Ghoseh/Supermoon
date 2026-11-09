# Legacy transition and safe inactive fallback on Steam 26540391

This plan binds public policy `1.2.0` to the exact
pair in recipe.json/BUILD_AND_PROVENANCE.md.  Keep NMS closed during staging, archival and restoration. Old RC DLLs must never reactivate on 26540391.

## Before any mutation

Verify exact executable/archive and all four installed identities, recording
absence too. Verify legacy policy, immutable manifest pin, backups and matching
tool bundle. Preserve whole verified original records/tools outside game/TEMP
before changing mutable journals. Back up current playlist.ini, Music (external
paths too), Cache, State, settings, logs and unrelated files. Preserve current
data; never restore old personal snapshots to satisfy historical preservation
checks. Archive latest logs before a launch; runtime replaces them.
Records bind canonical paths: copied real records must refuse relocated-path
mutation. Do not rewrite real records to make disposable tests work. Equivalent
fixtures must create honest records at their own paths.

## Matching legacy Disable, then separate new policy

1. Close NMS and keep other writers idle. Use only the matching legacy recipe,
   tool and record to inspect and Disable recognized resources. Unsupported
   executable must block old Install/Resume/Rollback, while matching Disable may
   remove recognized files. Legacy Python recovery is allowed for this transition.
2. Verify all four paths in INSTALL.md absent. A remaining bank is incomplete
   disable even when DLLs are gone. Stop on unknown bytes/corrupt records; never
   force overwrite or Rollback the old pair on 26540391.
3. Copy/read-back-verify the entire disabled legacy recovery root outside game/TEMP.
   Keep original and disabled snapshots, matching tools and older quarantined
   backups. After snapshots verify, archive the old root outside the game with
   exact inventory checks. Never merge roots/selectors or change path binding.
4. Confirm all four resources absent and no selected new record. From this exact
   extracted package run Status, Install (INSTALL confirmation), Status. Require
   candidate hashes and verified journal. Verify all new manifest `before` entries
   null/absent. Preserve the separate new recovery root and complete stage.
5. Verify personal/unrelated files unchanged; archive evidence. Only launch after verified completion. No source music/configuration restoration is required.

No automatic conversion is implemented. If matching legacy tools or ownership
evidence is unavailable, stop for exact file-specific recovery review. Do not
delete directories or fabricate/relabel records.

## Failure fallback is absence

- Before legacy archival, use only matching legacy Disable for recognized files.
  Verify all four absent before any new Install. Preserve records on failure.
- After new record creation, use exact matching Status. Resume can complete a
  known interruption; for abort choose Rollback only after verifying all-absent
  prior state, or Disable recognized candidate resources. Verify all four absent.
- Sharing failure can leave a bank after marker/DLL removal. Keep the game closed,
  retain stdout/stderr/status, release the identified other writer, inspect Status,
  explicitly retry matching recovery and verify absence. A partial state is not
  completed fallback. Do not launch with a leftover bank.
- Missing/corrupt backups, foreign current bytes, changed paths/policy or failed
  retry require a stop and ownership-qualified manual review. Preserve exact
  bytes, records and diagnostics. Do not edit hashes/selectors, delete records,
  copy old DLLs or assume reinstall repairs it. Any later approved quarantine
  must preserve and remove only exactly identified resources, activation marker
  first. No forced manual deletion is authorized by this plan.
- If NMS updates again, do not activate/resume/rollback. Matching Disable can
  remove recognized resources; verify absence, then seek review.

Safe inactive verification means filesystem absence, not a journal label, at:

```
Binaries/winmm.dll
Binaries/Supermoon/supermoon_playlist.dll
Binaries/bytebeat-playlist-experimental.enabled
GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk
```

Keep directories, personal files, records and unrelated mods. This proves only
Supermoon's four-resource inactive state, not other mods/game health. Retained
v1.0.3/RC DLL backups are historical evidence, never a fallback on 26540391.
Restoring an older game build is outside this plan. Leave Supermoon absent if
staging fails. Setup plus external archival is not one atomic transaction;
power-loss recovery is unproved and independent verified backups remain necessary.
