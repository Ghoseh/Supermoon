# Changelog

## v1.1.0 — prepared release artifacts; upload approval pending

The owner selected v1.1.0 for significant audio functionality beyond v1.0.2 and
approved **Ready with Caveats** for the limited desktop scope, subject to final
publication verification. Retain the exact Phase 6-tested `1.0.4-rc1-private` DLL
pair. The public recipe/recovery policy is now `1.1.0`; the installed RC record,
matching RC tools and private archives remain unchanged. See
[the version policy](docs/V1_1_0_VERSION_TRANSITION.md).

### Audio features beyond v1.0.2

- Carry forward intent-driven native soundtrack suppression through quiet/silent
  custom passages and internal EOF/refill, with the M2 final-Stop caveat below.
- Carry forward the locally generated Frontend bank workaround for Discoveries/
  menu ByteBeat persistence, including native ByteBeat behavior independent of DLLs.
- Carry forward exact Steam build 26385139 native integration and current indexed
  mod-settings support. These audio/integration capabilities first shipped in
  v1.0.3; this documentation pass does not newly implement them or alter that release.
- Retain existing playlist, WAV/MP3 preparation, normalization, filename display,
  shuffle/navigation keys and manual playback recovery.

### Reliability and usability improvements over the earlier release line

- Recover controller suppression after demonstrated transient failures with a newer
  validated generation; compensate audited stale-enable ordering cases.
- Require enabled desktop bank registration, bounded first-registration readiness
  and known loose-resource conflict checks while allowing unrelated mods, without
  required Steam launch arguments or automatic settings edits.
- Add staged four-file installation with durable backups, version-specific records,
  status/resume/qualified rollback/disable and unknown-file refusal. Use example-only
  INI packaging and preserve personal INI/Music/Cache/State/unrelated files.
- Prevent cache removal after incomplete discovery; protect the audited Init
  borrowed-read cleanup scope. Improve activation/capacity diagnostics and safe
  build-output reservation without native lifecycle or synchronization redesign.

### Qualification and accepted unresolved limitations

- Phase 6 owner-run desktop checks passed across two sessions: baseline/custom,
  quiet/owner-identified silent suppression, normal Stop restoration, menu cycles,
  Next/EOF, manual travel/reload recovery, fresh launch and normal exits. These
  are finite listening observations, not all-schedule safety or new endurance proof.
- **M2, original Medium:** three original final-release tests still fail. Native
  soundtrack volume may fail to restore after Stop under certain callback conditions;
  universal restoration is not guaranteed. No M2 gameplay failure is confirmed in
  reviewed evidence; likelihood/duration are not established. Original audit
  release-blocker classification remains historical, not an owner-originated label.
- **H1/U1:** isolated conditional cleanup self-deadlock remains reproducible; actual
  NMS gameplay reachability is unproven. A1 High conditional severity, original
  timeout and historical qualification-hold/U1 classifications remain. No gameplay
  occurrence is confirmed; normal exits do not prove exclusion.
- The owner explicitly accepted these unresolved risks for the limited desktop
  release, subject to final publication verification. Neither finding is fixed,
  harmless or a passing test. Phase 5 remediation remains **81 PASS / 3 M2 FAIL /
  1 H1 TIMEOUT / 0 ERROR** in each normal/applicable UBSan mode.
- Finite cumulative capacity, unmeasured long-session headroom/performance, manual
  Play after travel/reload, brief refill gaps, bounded diagnostics, bank conflicts
  and unsupported VR/other builds/mod managers/hot unload remain disclosed in
  [known limitations](docs/SUPERMOON_V1_KNOWN_LIMITATIONS.md).

The owner approved v1.1.0 artifact preparation and its separate public policy.
Local reproducible installation/source ZIPs and manifests/checksums are prepared
for final publication review. Setup/recovery suites pass 39 cases normally and
with applicable UBSan; 17 public CLI scenarios cover installation/update/recovery
and version refusal using genuine supported inputs as data in disposable fixtures.
No DLL rebuild, installed RC migration or publication occurred.
See [release notes](docs/RELEASE_NOTES.md).

## Historical Phase 5 and earlier entries

The entries below retain their then-current dispositions and validation scope;
they do not override the current owner decision or Phase 6 evidence above.

# Candidate 1.0.4-rc1-private (unreleased; private evaluation)

- Carries Phase 1 controller epoch/order recovery, Phase 2A desktop required-bank
  admission/registration, Phase 3 durable installation/recovery and Phase 4
  discovery/Init reliability and capacity/activation diagnostics.
- Conservative Phase 5 cleanup: correct stale empty-library wording, distinguish
  control pump/provider comments, remove an unused Python import. Runtime native
  hooks, ownership and synchronization are unchanged.
- New matching DLL identity pins/recipe/manifests; deterministic source packaging
  and installation documentation closure. No live INI or game/generated bank ships.
- Docs clarify ordinary desktop setup without launch arguments, unrelated-mod
  support/resource conflicts, staged updates, durable recovery/removal, Frontend
  effects, capacity/manual-recovery limits and historical qualification boundaries.
- A1-M2 final release remains a blocker (three original failures); A1-H1/U1 remains
  a qualification hold (original synthetic self-wait, gameplay reachability unproven).
  No guarantee of native restoration after every Stop. No installation, NMS session,
  Phase 6 or publication. Historical results below do not qualify this candidate.

# Changelog

## Unreleased — Phase 4 reliability and diagnostics

- Use explicit Win32 discovery completion/errors and prohibit cache removal on incomplete scans, including quota pressure; retain omitted reusable caches for a later clean scan.
- Protect owned Init borrowed reads with existing exception cleanup, preserving propagation, terminal failure and TLS/admission/latch restoration.
- Report cumulative retained lifecycle slots, bounded reservation refusals and degradation/restart guidance off callbacks without changing capacities or ownership.
- Report actual cold activation stages and exit/no-retry guidance after possible hook exposure, including preparation/publication refusals.
- Permit safe TEMP ancestor/sibling outputs while preserving protected/nested/reparse refusal. M2/H1 remain unresolved; no new runtime pair is qualified.

## Unreleased — Phase 3 installation and recovery

- Add a four-file installer with verified durable prior/candidate backups, immutable identity records, observed-state status, conservative resume/rollback/disable and a cooperative Windows writer lock.
- Preserve personal INI/Music/Cache/State and unrelated files. Build private package candidates with an example INI only; keep the sealed v1.0.3 ZIP unchanged.
- Replace executable-search-based `tasklist` admission with bounded Windows process enumeration; fail closed on errors and document exclusive access as a required boundary.
- Route legacy bank-only installation through the durable mechanism. Retain exact executable/DLL/archive/bank gates; refuse unknown current files and unqualified rollback combinations.
- Keep runtime code and original M2/H1 conditions unchanged. These installation changes do not qualify a new release or close those findings.

## Unreleased — Phase 2A required-bank admission

- Require a unique, unambiguous desktop-enabled Supermoon bank entry in addition to all existing identity gates. Preserve ordinary desktop launches without extra arguments; document VR as unsupported.
- Allow NMS's first-time registration to progress in the existing bounded startup window. Missing registration never enables activation; no game setting is created or changed automatically.
- Check known enabled loose `audio/windows/init.bnk` overrides, allowing unrelated mods and verified identical bytes. Document packaged/legacy and winning-resource inspection limits without claiming effective loading.
- Preserve original M3 fixtures/outcomes and separate global-only parser tests from production admission. M2 final release and H1 remain unresolved; no new release is qualified.

## Unreleased — Phase 1 partial controller remediation

- Cancel the affected playback generation on observed engine/liveness loss or a failed parameter call, allowing a strictly newer validated commit to recover without resetting terminal identity refusal.
- Recheck intent after parameter submission and make at most one immediate neutral compensation for an obsolete or failed enable. Preserve newer commits arriving during failure or compensation.
- Keep failed releases uncached for a subsequent qualified control update. **Final Stop/retirement release when callbacks cease remains unresolved; A1-M2 is not closed.** No new release or DLL pair is qualified.
- Retain the original Phase 0 assertions, add eleven controller failure/race cases, and leave M3 dependency-policy and H1 liveness tests unchanged.

## 1.0.3 - 2026-11-02

- Restore hook compatibility with NMS Steam build **26385139**, retaining exact executable, native-site and matching-payload gates.
- Keep ByteBeat playback audible through Discoveries and menu transitions.
- Suppress the native soundtrack throughout custom playback, including silent passages, and restore it after Stop or leaving the custom carrier.
- Accept bounded indexed entries in NMS's current mod-settings MXML without relaxing malformed/disabled-setting refusal.
- Generate the required audio override locally from the player's supported game bank; no game bank or playlist music is distributed.
- Confirm the quiet-passage, Stop/native-return, repeated Discoveries, fast-travel and save-reload checks with the owner. Preserve existing manual Play recovery and brief restart/navigation buffer gaps.
- Pass independent native/UI/audio reviews, recovery/publication and public regressions, and reproduce the exact owner-tested DLL pair. Existing resident shutdown limitations remain.

## 1.0.2 - 2026-11-01

- Adapt native audio, lifecycle, allocator, global and UI profiles to NMS Steam build **26295378** (7.11).
- Refresh executable, native-code, readiness and payload identities while preserving unknown-build refusal.
- Preserve playlist, normalization, controls, filename/time/progress and manual recovery behavior.
- Pass native/public/UI/identity checks and reproduce the exact installed DLL pair.
- Confirm custom music, Next, UI, logged automatic song advancement, fast-travel recovery, normal exit and playback after Steam relaunch. Warm activation took 0.605 seconds on the test machine.
- Retain the existing accepted findings and limits; no new four-hour endurance test is claimed.
- Menu-related temporary playback interruption and native soundtrack overlap remain unresolved. This release updates hook compatibility.

## 1.0.1 - 2026-10-28

- Restore compatibility with NMS Steam build **26271400** by updating native function, dispatch, lifecycle, allocator, global and UI hook profiles.
- Update executable, native-code and DLL identity checks. Unknown game builds continue to refuse activation.
- Preserve existing playlist, audio preparation, normalization, shuffle, keyboard controls and manual Play recovery behavior.
- Verify custom playback, Next, full-song automatic advancement, teleport/warp recovery, normal exit and playback after Steam relaunch. Warm activation took 0.651 seconds on the test machine.
- Pass updated native recovery, hook-publication, UI, startup and public regressions. Reproduce the installed DLL pair with the portable build wrapper.
- Support only the executable listed in [build and provenance](docs/BUILD_AND_PROVENANCE.md). Existing limitations remain; the original release's four-hour endurance result does not qualify this new DLL pair.

## 1.0.0 - prepared 2026-10-16

- Integrated WAV/Layer III MP3 playlist, disk-backed preparation/cache, optional loudness normalization, filename titles, keyboard navigation and shuffle cues.
- Event-agnostic source-ticket recovery: after native playback stops, manual Play can restore custom audio across repeated source lifecycles; no automatic resume.
- Independently reviewed startup XML, MP3 error propagation, shuffle synchronization, cache quota recovery, meter cleanup and allocator-last hook-publication corrections.
- Exact RC1 binaries retained after comprehensive audit, 4h00m24s Steam endurance and warm cache relaunch. No runtime rebuild for public packaging.
- MIT selected for original source; complete third-party notices and portable build instructions supplied. Accepted C1/C2/C3 and endurance observations remain open; see known limitations.
