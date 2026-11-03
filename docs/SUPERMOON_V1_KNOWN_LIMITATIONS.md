# Accepted unresolved limitations — v1.1.0

The project owner approved **Ready with Caveats** for the limited desktop scope
and explicitly accepted M2 and H1/U1 risks, subject to final publication verification.
The v1.1.0 public recovery policy retains the Phase 6 DLLs qualified as
`1.0.4-rc1-private`; the installed RC record/tools remain unchanged. Local public
artifacts await upload/release approval. See [qualification](QUALIFICATION.md)
and the [version policy](V1_1_0_VERSION_TRANSITION.md).

## M2 — original Medium, unresolved and explicitly accepted

Native soundtrack volume may fail to restore after Stop under certain engine
callback conditions: no later qualified release opportunity, last-source retirement,
or failed final neutral delivery without a later retry. The three original final
release regressions still fail with stopped intent and suppression applied. No safe
independent SDK release path has been established. A later valid update can repair
the tested parameter; an engine reset could change it, but neither is guaranteed
to occur or restore neutral on a particular schedule.

Normal Stop restoration passed finite Phase 6 listening checks. Restoration after
every Stop is not guaranteed. No M2 gameplay failure has been confirmed in the
reviewed evidence. Likelihood, frequency, duration and audible attenuation amount
are not established. No M2-caused crash, save corruption, persistent-file change
or unrelated-audio damage was demonstrated; that is not proof of all native safety.
The original Medium severity and historical independent-audit release-blocker
classification remain intact. The owner did not originate that classification;
current explicit acceptance changes release disposition, not the failed tests.

## H1 / historical U1 — conditional cleanup risk, unresolved and explicitly accepted

An isolated same-invocation cleanup deadlock remains reproducible: cleanup waits
for an admitted invocation whose own stack cannot release its lease until cleanup
returns. The original regression still times out. Actual NMS gameplay reachability,
native ancestor-lock order and all same-owner serialization schedules are unproven.
No gameplay occurrence is confirmed in the reviewed evidence. If reachable, cleanup
and dependent native work could hang; no likelihood or frequency is established.

Both Phase 6 sessions exited normally. This and historical U1 acceptance do not
prove safety. A1's original **High conditional** severity and historical qualification
hold are preserved; earlier U1 was described as conditional Medium. No timeout or
early-return fix was justified without native continuation/owner-lifetime evidence:
proceeding with destruction while calls remain admitted risks invalid lifetime.
No lifecycle redesign or speculative hook is part of the accepted release.

## Other compatibility, usability and performance limits

- Finite stores: 4096 Sources, 8192 Parameters, 4096 Events cumulatively per process,
  no reuse. Refusal/capacity diagnostics are bounded; native exhaustion handling,
  live headroom and all-session availability are unqualified. Exit normally/restart.
- Manual Play is required after native Stop/travel/reload; there is no auto-resume.
  Mono output, format limits, startup-only scan, brief restart/Next gaps and bounded
  diagnostic visibility remain. Hot unload/universal shutdown are unsupported.
- Discoveries uses a required locally generated/enabled bank; its Frontend volume
  change affects native ByteBeat too, independently of DLLs. Remove/disable both
  components. Suppression follows custom intent through quiet/silent/EOF passages,
  with M2's final release caveat. Phase 6 owner listening passed; native winning-
  resource identity was not directly observed and packed priority remains limited.
- Ordinary desktop Steam build 26385139 only. No launch arguments required;
  unrelated mods are allowed with exact-bank/resource-conflict restrictions.
  VR is unsupported and not reliably detected. Other stores/builds, mod managers
  and arbitrary packed/legacy override priorities are unqualified.
- Installation requires exclusive access with NMS closed. Cooperative locking is
  not hostile/concurrent namespace protection or proven power-loss durability.
  Cross-version automatic update/rollback is not supplied. Preserve personal files
  and verified durable records; unknown current files refuse automatic modification.
- C1/C2 are corrected in remediation source under owned tests; C3 stage reporting
  and U3 visibility improved. No broad lifecycle or performance rewrite was made.
  U2 upstream attribution and bridge/per-sample performance uncertainties remain.

Confirmed live behavior and unresolved edges are distinct: the owner heard normal
native baseline, custom/quiet/silent suppression, Stop return, menu transitions,
Next/EOF, manual travel/reload recovery and fresh launch. Brief refill gaps remain.
These two sessions are not new long-duration endurance or an all-schedule proof.
Installation preserves personal files; complete runtime scans can remove obsolete
managed cache pairs, separately from original music. Version-specific recovery
records and matching tools must stay intact; never rewrite them to claim compatibility.

The following text is retained historical v1.0 evidence. Statements such as
"unfixed" and earlier menu/overlap results describe those older pairs only.

# Supermoon v1.0 â€” known limitations and qualification status

## Background soundtrack overlap

NMS's native background soundtrack may become audible during quieter portions of
custom music. NMS uses ByteBeat audio-level-dependent attenuation, which can
release as custom audio gets quieter. The exact audible source and its full runtime
gain have not been conclusively identified. This remains an accepted v1.0 limitation;
v1.0 does not include a soundtrack-suppression fix.

## Menu-related interruption

The owner reports that opening Discovery or the pause menu can intermittently
interrupt custom audio and make native game music audible. Custom audio resumes
when the menu closes without another Play command. This is reproducible but not
every time. Its connection to background soundtrack overlap is under investigation;
v1.0.2 does not include a menu-audio fix.

## Native lifecycle and manual playback

Recovery responds to native audio lifecycle events, not gameplay-specific teleport
or warp triggers. A retired owned source ticket remains permanently retired; new
qualified native playback may use a different retained ticket. Travel may stop
native music, and Supermoon does not automatically resume after arrival. Press
native Play manually to restore custom playback. Same-source transitions that keep
native playback active need not stop music. Repeated manual recovery is a required
feature, not an accepted terminal-retirement limitation.

Owned identities protect admitted source invocations; they do not prove closure of
all upstream native work or every native lock/cleanup schedule. Source, Parameter
and Event stores have finite process-wide capacities without reuse; exhausted or
ambiguous identities refuse safely. Very long/high-count headroom and worst-case
callback/scan deadlines remain unmeasured. Callback-visible state stays resident.
Hot unloading, exhaustive native draining, clean native shutdown on every schedule
and guaranteed base-speaker routing are not established.

## Accepted audit findings

The comprehensive independent audit found no confirmed Critical/High release
blocker for controlled endurance. The owner accepted C1 (Medium: interrupted
discovery can omit tracks and evict reusable decoded caches), C2 (Low: synthetic
invalid-parameter Init/TLS cleanup gap) and C3 (Low: partial-publication diagnostic
ambiguity) for v1.0. They remain unfixed. No original-song loss, ordinary valid-caller
C2 crash or new incompatible native Free route was established. See
ACCEPTED_AUDIT_FINDINGS.md for triggers, impact and separate U1â€“U3 uncertainties.

Use a stable, preverified local library and inspect actual prepared inventory.
After any refusal with possible hook exposure, exit NMS normally; never hot-retry
activation or unload individual wrappers. Ordinary gameplay changes to settings,
saves, logs and timestamps are authorized user activity and must not be reverted
solely to match historical hashes.

## Audio, controls and display

Supported WAV/MP3 inputs are prepared as mono; stereo output is not provided.
WAV support is limited to the implemented encodings/formats. Correctly finalized
RIFF/data lengths are required: usable PCM samples can still be rejected when
headers contain unfinished or inconsistent lengths. Re-exporting through an editor
such as FL Studio may produce compatible files. Supermoon does not repair malformed
headers. See SUPPORTED_FORMATS.md; not every WAV file is supported.

OGG/FLAC, Layer I/II/free-format MP3 and changing MP3 formats are unsupported.
Unrecognized gapless metadata may retain padding. MPEG CRC is not authenticated;
all semantically corrupt finite audio cannot be distinguished from valid input.
Targeted guard/UBSan/reference tests are not an all-input safety proof, and copied
machine code is not sanitizer-instrumented.

Normalization uses constant gain and conservative measured peaks, not dynamics
limiting or a guarantee about every native gain path. Short queue/switching pauses,
rendered UI variants and focus/overlay interactions require observed qualification.
Filename titles are bounded to 255 UTF-8 bytes and can truncate; failed UI guards
retain native display. Runtime library refresh and cross-session position/history
are not implemented. Foreground/capture keyboard heuristics cannot guarantee all
overlay/binding combinations. Default shortcuts use Ctrl+Shift, not bare F6â€“F9.

Shuffle toggles preserve already queued PCM; up to 16,384 published frames can
delay future order at 24 kHz, with longer wall-clock delay while paused. Notification
events may coalesce. Cache integrity checks are not authentication or a guarantee
against every filesystem/power failure. Original source music remains separate
from managed decoded cache files.

## Startup, diagnostics and remaining live qualification

Startup parses bounded UTF-8 MXML and rejects malformed/ambiguous logical settings.
Settings are sampled before activation, not continuously monitored. Edits after the
last sample are not automatic cancellation. Executable/payload/profile/marker gates
remain separate; a compatibility Yes does not bypass native identity checks.

The exact final F6 DLL pair passed a 4h00m24s Steam session and a separate warm
relaunch. Repeated travel/manual Play recovered custom playback, with normal exit
and verified archives/rollback. The later session phase was unattended; it is not
four hours of continuous human listening. See QUALIFICATION.md.
Absence of recorded races is not proof they are impossible. Zero-produced callbacks
alone do not prove audible starvation. Detail buffers are finite; missing evidence
must be reported, not reconstructed. Footer/worker-scope markers do not prove native
draining or detached-writer completion. Verify process exit, released writer handles
and stable inventories before archival/relaunch.

## Historical preservation exception and public packaging

An inherited fixed test-output path overwrote one historical generated telemetry
fixture. Its original SHA-256 is
`a0f8cbe516a32bebc2ea7940d6c5beecb0e1a87cb031329fe0c81b0a18e13b85`.
No exact copy was recovered in the documented bounded search. Original bytes remain
unrecoverable; no reconstruction was attempted. The overwritten version is preserved
separately. Current preservation checks do not erase that historical exception.
All new verification outputs use fresh isolated disposable directories.

## Endurance observations retained for v1.0

- Intermittent pause-menu playback persistence was reproduced by the user; custom
  playback remained recoverable. The native/mod cause is unconfirmed. No fix is claimed.
- Fuzzy vocal distortion was reported on one test track. Its prepared PCM was finite
  and below full scale, with no new recorded underruns near the report. This does
  not measure the native/game/OS output or establish the recording's quality.
  No confirmed clipping or decoder defect was demonstrated; the observation remains open.
- Detailed provider records filled their 262,144-record capacity during endurance.
  Full-session health/resources continued, but later detailed events may be absent.

The public package excludes original private audit, native research, music and
rollback archives. Their originals remain preserved. Exact-package privacy checks
are required for each future distribution; see PUBLIC_DISTRIBUTION_PRIVACY.md.
