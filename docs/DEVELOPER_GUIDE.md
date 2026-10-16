# Implementation map and contracts

This guide describes the comprehensively audited F6-corrected implementation.
Statements about code flow are source contracts; copied-native fixtures and short
live observations have narrower scopes. The accepted noncritical findings and
unverified host contracts are recorded in ACCEPTED_AUDIT_FINDINGS.md. Documentation
cleanup changes none of them and does not establish every native runtime behavior.

## Entry points and startup

`source/supermoon_playback.cpp` selects the playback defines and includes the shared
adapter implementation. `winmm_bootstrap.cpp` is the separate proxy entry point.
Loading the payload alone does not call its activator; loading the proxy schedules
a worker that may activate the payload after all gates pass. DllMain does not wait,
resolve WINMM exports, hash files, inspect game objects or publish hooks itself.

The worker derives paths from the executable location, preflights OS forwarding,
checks the exact enable marker and host path, compatibility decisions, pinned
executable, mod settings, payload identity, existing modules, readiness code and
original dispatch cells. It parses settings before readiness polling and again
before loading the payload. A compatibility Yes does not supply an unknown native
profile or bypass executable/site checks.

`startup_policy.hpp` accepts a bounded UTF-8 MXML subset: one non-self-closing
GcModSettings Data root and balanced Property elements, optional supported XML
declaration/comments, at most 65,536 bytes, 32 open elements and 16 attributes per
element. It is not full XML/schema support. Unsupported markup, invalid UTF-8 or
XML characters, malformed references, missing/unknown/disabled settings and
structural ambiguity refuse authorization. Decimal and lowercase-x hexadecimal
references are decoded once; either hex digit case is accepted. The name, enabled
value and root template use logical ASCII equality. A nested DisableAllMods alias
or any second logical match refuses, including identical false settings.

The settings reader owns a 65,537-byte stack buffer, rejects over-limit/read errors
and passes an explicit-length view whose lifetime ends inside that call. Each
read/share handle closes before returning. The second read samples policy after
readiness; edits after that sample are not continuously monitored or latched
cancellation. The final independent F1 review documented this pre-existing timing
limit without observing a live late-edit race. It did not identify a parser blocker.

## Native boundary, tickets and resident lifetime

The production recipe defines `ADAPTER_RECOVERY_BUILD`. `native_clone.hpp` selects
the generated recovery layout; the older native layout and legacy branches remain
for non-recovery/self-test paths. Pinned executable, function-byte, native-pin and
dispatch checks are distinct. `NativeClone` relocates private copies, widens branches
where required, checks signed rel32 reach and registers stable unwind table storage.
Native instruction bytes stay intact. The seven bridges preserve their pinned
register/flags/SIMD/stack contracts; the source-plugin ABI is build-specific.

Activation publishes 15 lifecycle/allocator cells before potentially long cold
preparation so source creation can be observed. Cleanup is first; owning allocation
is last, after all 14 dependent hooks and their protection restores. Free, Source
and parameter destruction/Term, Init/Execute/Format, parameter mutation and owner
hooks must exist before an allocator can expose Registry storage. A failure before
allocator exposure keeps genuine native allocation and foreign-pointer forwarding.
If the allocator exchange succeeds but its own protection restore fails, dependency
wrappers remain compatible and resident, but activation still fails. Never roll back
those wrappers, retry or hot-unload after possible exposure.

During early lifecycle publication `engine_ready` is false: wrappers avoid Core/PCM
access while forwarding native work and recording lifecycles. Preparation owns local
payload resources until adoption. Once adopted, the Core/Engine remain resident;
the Core pointer/validator are established before the release/acquire readiness gate.
Four later provider/update/pool cells publish individually, with provider last;
cleanup is already owned by the lifecycle hook. UI activation follows successful
audio publication. These exchanges are not one transaction. A later refusal may
still leave hooks exposed even if its log omits explicit partial-publication wording
(C3); normal process exit is required.

Only exact pinned source/parameter/Clone factory callers and sizes receive owned
slots. Other allocations and foreign object methods retain native ownership. Source,
Parameter and Event slots are bounded and never reused during process lifetime.
The source address is therefore a stable owned ticket, unlike a reusable native
owner address. Numeric PlayingID reuse and multiple source claims are ambiguous
and close admission; neither pointer nor ID equality is incarnation proof.

Owner Init creates an append-only Event; the PostEvent return bridge publishes its
PlayingID. Source Init borrows caller-live context to obtain an ID and associate a
qualified Event, then native Init/Execute use resident context interfaces and stable
parameters. The accepted C2 parameter snapshot is before the finally-protected
scope, so invalid borrowed-parameter faults are an explicit cleanup exception.

Execute acquires its Source/event admission lease before the copied native lookup,
provider and tail. TLS carries that identity through the invocation; `__finally`
restores prior TLS, releases the lease and clears executing on guarded normal/C++/
SEH exits. Concurrent use of one Source is refused. Owner cleanup/reinitialization
closes event admission and waits for already-admitted owned scopes before this
wrapper calls original native cleanup. Term retires without waiting. No retired
ticket is reopened, and native storage is not reclaimed by a guessed timeout.
This owned boundary does not prove universal native task closure, lock order or
upstream delayed-work attribution (U1/U2).

Frame helpers validate stack index +0x80, output +0x188, request +0x190 and produced
pointer +0x198 against borrowed provider arguments while native synthesis holds its
lock. The owned ticket reaches Core only with a matching live lease. Native stream
addresses remain control/UI lookup values; they are not callback incarnation proof.
Resolver/setter provenance and copied carrier key/payload identity validate native
Play before `commit_endpoint` arms a live source. Carrier clock, commit generation,
playlist serial, queue revision and consumed PCM position have different purposes.

Retirement disables custom consumption and pauses the resident Engine without
terminating its worker. A later qualified native Play may commit a different owned
ticket. Teleport, warp, portal and reload are possible triggers, not hardcoded event
branches. No automatic resume occurs; if native playback stops it stays stopped
until started again. A transition retaining native playback may continue playing.
The retained legacy `commit`/`retire` native-pointer path is terminal and cannot
rebind; it does not describe production source-ticket recovery.

Callback-visible clone, unwind, Registry, Core/Engine and UI state stay resident.
Worker stop, scope exit, thread join, capture footer, process termination and native
callback/task closure are distinct observations. None of the first four proves
the latter. Hot unload, exhaustive native draining, all shutdown schedules and
worst-case callback deadlines remain unqualified.

## Queue, controls and notifications

`Engine` has one serialized PCM consumer and one disk worker. Consumer read index
and intra-block offset are consumer-owned; reader, write index and Order/history
are worker-owned. Release/acquire publication exposes complete immutable blocks;
consumer release permits reuse. Queue storage is 16 × 1,024 float frames (65,536
sample bytes), not a preloaded whole soundtrack. Callback APIs do not decode,
allocate, read files, sleep or wait on a lock. That source contract and audited
allocation observations do not establish real callback deadlines on every system.

Pure shuffle toggles update future Order state and preserve queued PCM, serial,
revision and consumed position. Already published tracks retain their sequence.
Up to 16,384 queued frames represent about 0.683 seconds at continuous 24 kHz
consumption; pausing consumption can extend the wall-clock delay. An EOF-selected
but unpublished next track is reselected from the last queued state. Navigation,
restart and failed-track quarantine invalidate revisions; the consumer retires
stale blocks rather than the worker reclaiming consumer-owned slots.

Previous uses a request-time consumed-PCM snapshot: within the first three seconds
it requests previous history; beyond that it restarts the current selection unless
other navigation supersedes it. F9 advances through a fresh shuffle choice without
changing persistent mode. Pure mode persistence failure records failure and leaves
that command unapplied; activity/restart changes can suppress a delayed cue after a
preference save. Notifications are a latest-commit mailbox, not an audible-event
queue; rapid events may coalesce. Cue scale is computed using the track peak bound
and current native factor; unsafe cue contribution mutes rather than changing music.

## Preparation, decoder and cache

`wav_pipeline` handles strict little-endian RIFF PCM8/16/24/32 or float32/64,
supported extensible variants, 8–384 kHz and mono/stereo input. RIFF/data lengths
must be correctly finalized; valid PCM samples alone do not make an inconsistent
container acceptable. The runtime does not repair headers. See SUPPORTED_FORMATS.md. MP3 uses the
production low-level vendored dr_mp3 decoder through `mp3_reader.cpp`, not its
general high-level file APIs. Layer III MPEG1/2/2.5 is supported within explicit
header/size/rate/channel/count limits. Layer I/II/free-format or format-changing
streams refuse. MP3 compressed size is capped at 512 MiB, frame count at one
million and recognized tag size at 16 MiB. Xing/LAME-family trimming is conditional;
unknown metadata may retain padding. CRC authenticity is not established.

Mandatory granule/scalefactor/Huffman budgets and physical bounds are distinct
from safe speculative count1 lookahead/stuffing. The vendor patch propagates sticky
failures, clears whole-frame output and returns zero. The wrapper requires the full
frame count and exact format before exposing samples. Successful reservoir/QMF
state persists across frames; clearing temporary scratch is not a reservoir reset.
Some semantically corrupt finite audio may remain indistinguishable from valid
content. Do not turn targeted guard/UBSan results into an exploitability verdict.

Preparation converts to 24 kHz mono float with a 64-tap sinc resampler when needed.
It uses disk-backed intermediates and extra peak/optional loudness scans, not just
two total file reads. A constant preparation gain implements optional -16 LUFS / -1
dBTP defaults, a conservative 0.5 dB margin and post-gain verification; no compressor,
limiter or dynamics processing is introduced. Normalization-off retains the older
negative-only reconstructed-peak headroom. The production meter is the separately
pinned `ebur128_peak.c`, with its qualified low-rate filter/cleanup, not the unchanged
vendor reference C. All vendor initialization/measurement is serialized off audio
because initialization rewrites shared histogram tables. Zero/reflected endpoint
measurements are conservative checks, not a proof of every playback reconstruction.

Discovery currently has accepted C1: interrupted enumeration can silently return
a partial path list. Empty diagnostics therefore do not prove complete discovery;
obsolete-entry eviction can discard reusable decoded pairs. Original music is not
deleted. A later clean scan rebuilds omitted entries. Use a stable verified library
for controlled endurance; this text does not change the eviction gate.

Persistent cache entries include source path/content/recipe/settings identities,
source file metadata, full PCM hash and checked metadata body. WAV recipe remains
3; MP3 additionally hashes its bounds2-granule-errors recipe. Source and referenced
PCM leases deny ordinary write/delete opens. Hashes detect changes; they do not
authenticate attacker-controlled files. Successful PCM flush precedes checked
metadata publication through a pending file/rename. Recovery revalidates complete
pairs and handles orphan/pending data; arbitrary hardware/power failures are not
exhaustively qualified. Per-source failure refreshes both quota bytes and eviction
inventory so newly created managed orphans are considered during the same run.
Leases still veto deletion, and unmanaged files consume quota without becoming
deletion candidates. Same-run admission is not promised through active leases or
external directory mutation. Hash/format/normalization work is startup-worker work.

## UI, logs and historical helpers

Filename titles are precomputed from owned basenames. `time.cpp` freezes one copied
title/TIME/progress tuple for the matching native rendering call, checks owner/key/
site/object contracts and taints nested scopes. TLS cleanup preserves original
exception propagation; native text remains fallback on failed guards. The register
progress bridge changes presentation data, not native duration, layout/controller
memory or audio position. UI-side hashing/read operations are not audio callbacks.

Detailed journals have finite slots/byte budgets; independent aggregates continue
within their own budgets. UI uses a six-hour writer limit, 4 MiB detailed and 32 MiB
total budget. Adapter legacy output has 64 MiB, health 64 MiB and events 32 MiB caps.
Counters are independently sampled; missing/dropped detail is not reconstructed.
These diagnostics are observation, not audible-quality or native-shutdown proof.

Retained `playlist/integrated_payload.hpp`, `control_policy.hpp`, `windows_controls`
and the 24 kHz-only `wav_reader` utility are historical/support paths. Production
uses `supermoon_playback_payload.hpp`, persistent cache and WindowsKeyboard. Test
mains and preparation CLI are not DLL entry points. Keep their historical failures
and provenance; do not copy their old output paths into a live qualification run.
