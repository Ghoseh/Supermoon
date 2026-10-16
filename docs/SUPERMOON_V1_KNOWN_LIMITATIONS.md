# Supermoon v1.0 — known limitations and qualification status

## Background soundtrack overlap

NMS's native background soundtrack may become audible during quieter portions of
custom music. NMS uses ByteBeat audio-level-dependent attenuation, which can
release as custom audio gets quieter. The exact audible source and its full runtime
gain have not been conclusively identified. This remains an accepted v1.0 limitation;
v1.0 does not include a soundtrack-suppression fix.

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
ACCEPTED_AUDIT_FINDINGS.md for triggers, impact and separate U1–U3 uncertainties.

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
overlay/binding combinations. Default shortcuts use Ctrl+Shift, not bare F6–F9.

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
