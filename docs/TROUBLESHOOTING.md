# Troubleshooting — v1.1.0

The owner approved Ready with Caveats and explicitly accepted unresolved M2 and
H1/U1 risks for the limited desktop scope, subject to final publication verification.
The v1.1.0 public policy retains the exact Phase 6-tested `1.0.4-rc1-private` DLLs.
The installed RC remains under its unchanged record/tools; see
[version-specific recovery](V1_1_0_VERSION_TRANSITION.md).
See [qualification](QUALIFICATION.md) for finite live results and unchanged failures.

**Inactive/original track/no custom audio:** check exact executable and matching
DLL identities in `build/recipe.json`, the marker, mod settings and folder layout.
Select Supermoon in the personal player. Cold cache preparation delays availability.
Inspect `Supermoon/bytebeat-startup-latest.log` and `preparation-latest.log`.
After refusal, exit normally before another attempt; never hot reload/swap DLLs.

**Missing/disabled/wrong bank:** generate through the staged installer in INSTALL.md
and enable the registered Supermoon entry in NMS's desktop mod settings. Missing
registration is pending, never permission to activate. The worker allows up to
120 seconds; `audio_registration_pending_exit_normally_and_restart` means exit
normally and restart after NMS saves its entry. Do not hand-create an MXML record.
Known disable/malformed settings or wrong bank refuse. No launch arguments are needed.

**Other mods:** retain unrelated mods. Resolve a specifically identified incompatible
`audio/windows/init.bnk` override or foreign `winmm.dll`. Loose conflict checks cannot
prove the winning resource in opaque/legacy packages. Blind priority changes and
arbitrary mod-manager deployment are unqualified. VR is unsupported.

**Setup failed or interrupted:** keep NMS closed and other writers idle. Run the
trusted setup tool's `status`, keep `.Supermoon-Recovery` and its record ID, then
choose `resume`, qualified `rollback` or `disable` as documented in INSTALL.md.
`REFUSED_OR_INCOMPLETE` can mean a bank was already replaced. Do not overwrite an
unknown file, discard the first backup, edit record hashes or delete a whole folder.
A new recipe does not authorize overwriting old-version DLLs or importing a record.
The installed RC record must stay with its matching RC tools. The v1.1.0 recipe will
refuse that record even if DLLs match; do not rewrite its release/hash or run a new
tool to force recovery. No installed transition is authorized by this documentation.

**Native soundtrack fails to return after Stop:** M2 is an accepted unresolved
Medium limitation. Logical Stop does not guarantee a subsequent legal release/
retry opportunity. Another qualified update could repair the parameter; a reset
could change it, with no timing or restoration guarantee. Normal Stop restoration
passed live checks; no M2 gameplay failure is confirmed in the reviewed evidence.
If an unexpected failure occurs, stop testing and preserve available diagnostics
before relaunch overwrites them. Exit normally when possible. Normal exit/restart
is the conservative process boundary, not proof of universal repair. Do not hot
reload, add SDK callers or repeatedly change controls to conceal a failed check.

**A freeze occurs:** H1/U1 is an accepted unresolved conditional risk, with A1 High
conditional severity. Its isolated self-wait remains reproducible; gameplay
reachability is unproven and no gameplay occurrence is confirmed. Do not attribute
every freeze to H1. Preserve available evidence; do not swap DLLs/banks or attempt
live repair. A timeout does not authorize native destruction. Both normal exits
and earlier listening do not exclude the condition.

**After travel/reload:** press Play manually if native ByteBeat stopped. No automatic
resume exists. Persistent failure after manual Play is not an accepted guarantee.

**Missing songs/cache pressure:** top-level supported WAV/MP3 only; restart after
changes. Incomplete enumeration now blocks cache removals and retains omitted pairs;
a later complete scan can recover them. This is not an atomic directory snapshot.
Keep original music stable and allow space for PCM and temporary preparation passes.
CacheMiB defaults to 3072 and includes unmanaged files/scratch, not all diagnostics
or original music. Do not remove active cache files. Malformed WAVs must be
re-exported separately; Supermoon does not repair source files.

**Capacity degraded:** `# recovery_capacity` reports cumulative slots/refusals,
ID reuse, late calls, overlap and cleanup waits, subject to log budgets. Slots do
not recycle. Exit normally/restart for a fresh process; no hours-to-exhaustion or
all-session headroom is established. Missing logs are not proof of healthy state.

**Quality/keys/display:** output is mono; constant-gain normalization is not a limiter.
Brief refill gaps remain. Ctrl+Shift is required for F6 through F9. Focus/overlays
can interfere and failed UI guards retain native display. Historical vocal/menu
observations are not new candidate validation or confirmed decoder defects.

Logs can include personal paths/song names. Share only reviewed, redacted minimal
excerpts. Never post music, caches, saves, credentials or full diagnostic archives.
