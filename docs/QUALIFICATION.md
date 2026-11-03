# Current qualification — v1.1.0

**Ready with Caveats for the limited desktop scope**, approved by the project
owner subject to final publication verification. M2 and H1/U1 are explicitly
accepted unresolved risks. Original Medium M2 severity, three failed tests and
historical audit release-blocker classification remain. H1 retains A1 High
conditional severity, the isolated timeout and earlier U1 history. Neither is
fixed, proven harmless or a confirmed gameplay failure in reviewed evidence.

Public v1.1.0 retains the exact DLLs tested as `1.0.4-rc1-private` from production
source checkpoint `50a8577`. No DLL rebuild occurred. The public recipe version
is `1.1.0`; the installed RC record/tools and private ZIPs remain unchanged.
Local artifacts await explicit upload/release approval under the
[approved version policy](V1_1_0_VERSION_TRANSITION.md). Current pins are in
`build/recipe.json` and `RELEASE_DLL_SHA256SUMS`.

## Supported scope

Windows x64 ordinary desktop, Steam build **26385139** only; `NMS.exe` 89,535,560
bytes, SHA-256 `a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.
No extra Steam launch arguments. Required locally generated/enabled bank and
matching DLL/marker identities remain mandatory. Unrelated mods may remain enabled
subject to proxy/init.bnk conflicts. VR is unsupported and not reliably detected;
other builds/stores, arbitrary packed priorities, mod managers, multiplayer and
an OS compatibility matrix are unqualified. See [installation](../INSTALL.md).

## Offline evidence for these exact binaries

Phase 5 linked both DLLs with pinned Zig 0.15.2, compiled all 23 recipe units and
the meter, and reproduced both DLL identities in seed plus two strict wrapper
builds. Both private archives reproduced; source/member/manifest correspondence,
notices/privacy selection and genuine bank derivation were verified.

| Final Phase 5 check | Normal and applicable UBSan results |
| --- | --- |
| Public tests | 11 suites plus 595-check corpus; 25 successful receipts per mode |
| Remediation | **81 PASS, 3 M2 FAIL, 1 H1 TIMEOUT, 0 ERROR per mode**; runner exit 1 |
| Reliability | 26 C++ and 6 Python PASS per mode |
| Installation | 39 PASS per mode, repeated after documentation member selection |
| Actual candidate-byte installation fixtures | 7 PASS; no game/DLL execution |

Phase 6 preparation added 18 passing migration/recovery fixtures using actual
old/new DLL and bank bytes as data with synthetic executable/personal sentinels.
Four cross-volume quarantine-helper fixtures passed before the approved install.
Actual installation verified all four resources and durable recovery records.

Python, the C meter, assembly and native NMS code are not UBSan-instrumented.
No observed sanitizer diagnostic on completed paths is not an all-input proof;
H1's timeout is an actual failed liveness result, not a completed clean run.
The original M2/H1 assertions and conditions remain unchanged. No added repair
callback, XFAIL conversion or expected-failure whitelist makes them green.

## Phase 6 owner-observed live results

Two controlled sessions launched normally through Steam with no additional game
arguments. The owner operated/listened; passive diagnostic records were preserved.
All requested functional checks passed by owner report:

- Native soundtrack before custom Play; normal/loud custom audio with no overlap.
- Quiet and owner-identified digitally silent passage suppression; normal ambience/FX.
- Four explicitly reported Stop checks and five repeated Play/Stop cycles restored
  native soundtrack normally in the reported windows.
- Five Discoveries and five pause-menu cycles; three Next advances with brief gaps.
- Two requested natural endings/transitions; continued playback and suppression.
- Travel and save reload stopped playback; manual Play restored it normally.
- Fresh-launch custom playback/Stop; both sessions exited normally with OS process
  absence verified. No crash, activation failure or unexpected audio was reported.

Digital silence was not independently PCM-measured. After Step 14 the owner pressed
Play again; later enable/advancing-PCM samples describe resumed playback, not a
contradiction of the earlier Stop observation. SDK success is submission evidence;
audible reports are a separate evidence layer, not inferred from those commands.
Disk bank/settings identities do not directly identify the native winning resource.

Session 1's final sampled health prefix was 1,711,922 ms; session 2's 182,031 ms.
These are sampled spans, not exact process lifetimes or a new four-hour endurance.
Session 1 recorded 28,273 empty PCM frames and a maximum sampled consecutive run
of 512; owner heard brief refill/navigation gaps. This is not zero-underrun or
a universal audible-gap bound. Empty total stayed unchanged through the two
requested EOF transitions. Session 2 had zero empty frames. Both had zero recorded
exceptions/faults/failed tracks/stale frames/ring drops; one late call was rejected
in session 1. Capacity snapshots were 3/7/3 then 1/1/1, without pool refusals.

Music, playlist.ini and mod-settings bytes stayed unchanged. State and diagnostics
changed normally through execution. One unreferenced managed PCM/meta pair was
removed at startup, consistent with documented complete-scan eviction; that cause
is an inference, not a traced deletion. The original pair remains in durable backup.

## Remaining limits and acceptance boundary

M2 may leave soundtrack suppression applied after final Stop/retirement or failed
neutral delivery when no later qualified callback/retry occurs. Normal restoration
passed live tests; universal restoration is not guaranteed. A later valid update
can repair the fixture, or a reset could change state, without a delivery/timing
guarantee. No confirmed M2 gameplay failure or demonstrated M2 crash, save corruption,
persistent-file change or unrelated audio damage exists in the reviewed evidence.
Likelihood, frequency and duration are not established.

H1's isolated cleanup self-wait remains reproducible. Actual NMS reachability and
ancestor-lock/serialization contracts are unproven. If reachable, cleanup/dependent
native work could block indefinitely; no gameplay occurrence was confirmed. Both
normal exits and historical U1 acceptance do not prove that schedule impossible.
See [known limitations](SUPERMOON_V1_KNOWN_LIMITATIONS.md) for the retained severities.

Finite cumulative lifecycle capacity/native exhaustion response, representative
long-session headroom, worst-case callback performance, real native partial-
publication/cleanup failure schedules and U2 upstream attribution remain unqualified.
Manual playback recovery, brief refill gaps and bounded diagnostics remain.
Hot unload, exhaustive native draining and universal shutdown are unsupported.

The owner's explicit acceptance permits a limited release judgment while leaving
the original final-release/liveness criteria technically unmet. It supplies no
implementation fix or probability estimate. No publication is approved yet.

The private Phase 5, Phase 6 preparation/installation/live and final risk-assessment
reports retain raw evidence and exact hashes outside the release tree. Public
source/member manifests and checksums are deliberately refreshed for v1.1.0.
`PACKAGING_VERIFICATION.json` separates retained runtime evidence from new public
setup/package checks; the Phase 5 summary is preserved as
`V1_0_4_RC1_PRIVATE_PACKAGING_VERIFICATION.json` and in sealed private evidence.

## Public artifact verification

The v1.1.0 setup/recovery suites pass 39 cases each normally and with applicable
UBSan on the owned process helper; Python is not instrumented. Seventeen public
CLI scenarios pass against disposable copies of the supported executable/native
archive used as data only. They cover clean install/status/update, rollback,
resume/disable, interruptions, unknown-file and backup refusal, and public/RC
policy rejection. No game/DLL instruction executes. These are new setup checks,
not new M2/H1, C++ runtime, listening or endurance runs.

Final archives are assembled twice and checked for reproducible identities,
member/manifest/checksum/path/case/CRC/source correspondence, documentation links,
privacy selection and notices. The external release receipt and `SHA256SUMS`
identify exact final ZIPs without a circular embedded ZIP hash. Upload approval
remains separate. No installed record is migrated or rewritten.

Everything below is retained historical evidence for named earlier pairs, including
phrases describing their then-current state. It must not be presented as validation
of the retained RC pair or overwrite the current assessment above.

# Historical version 1.0.3 qualification

This is the historical sealed-pair qualification. Unreleased remediation source,
including [Phase 4](REMEDIATION_PHASE4.md), does not inherit it. The new private pair has offline evidence only; M2/H1 remain unresolved.

The exact DLL pair supports Steam build 26385139. Independent native-profile,
UI and recovery reviews, current-native fixtures, executable identity rejection
cases, all eleven public suites, the indexed-settings guard-page corpus, normal
and UBSan UI coverage, and startup/parser UBSan tests passed. Two portable builds
reproduced the owner-tested DLL hashes. The local bank generator passed derivation,
installation, backup, idempotence and conflict/identity-refusal tests on an owned
temporary installation tree.

On 2026-11-02 the owner confirmed native music before Play, suppression through
silent custom passages, native music returning after Stop, continued playback
through repeated Discoveries menu transitions, Next, and custom recovery after
fast travel and save reload. A further Stop after those transitions restored
native music. NMS is closed; the accepted pair remains installed.

The finite session recorded two natural EOF transitions and no callback exceptions,
faults, failed tracks, stale frames or dropped diagnostics. It also recorded
20,943 empty frames across manual restart/navigation transitions, with a maximum
512 consecutive frames. Existing restart/Next refill gaps remain; this is not
zero-underrun qualification. No new four-hour endurance, multiplayer certification,
universal shutdown or live-unload result is claimed. See MIGRATION_STATUS.md and
SUPERMOON_V1_KNOWN_LIMITATIONS.md. Historical v1.0.1 and v1.0.2 evidence does not
qualify this pair.

# Original release qualification (historical)

These results apply to the original v1.0.0 pair. For the historical build 26271400 repair, see V1_0_1_MIGRATION_STATUS.md. Current version status is in MIGRATION_STATUS.md.

# Qualification summary for the exact shipped pair

Comprehensive independent review found no confirmed Critical or High blocker.
F1â€“F6 passed focused independent verification. Accepted C1 Medium and C2/C3 Low
remain described in ACCEPTED_AUDIT_FINDINGS.md, along with unverified U1â€“U3 contracts.
This public summary does not replace or redistribute the preserved private reports.

On 2026-10-16 the exact RC1 pair completed 4h00m24s of Steam-online NMS operation
from confirmed custom playback to normal process exit, followed by a separate
successful warm Steam relaunch and normal exit. The user confirmed the requested
travel/manual-Play transitions. All 27 prepared songs appeared in consumed snapshots;
65 completed-track increments were recorded. The later phase was AFK, not continuous
human listening. No crash, recorded callback exception, faulted/failed track,
stale-available frames or sampled same-track/serial/revision position regression
was recorded. Zero-produced callbacks alone were not classified as audible starvation.

Cold startup rebuilt 27 entries; warm startup reused all 27 and rebuilt none.
Observed activation spans including preparation were 115.221s cold and 0.611s warm
for this library/system, not performance guarantees or isolated decoder benchmarks.
Whole-game memory increased during active play and plateaued late (~15.412 GiB
private bytes); last-hour fitted slope ~0.105 MiB/hour. These are whole-NMS trends,
not an isolated mod leak measurement or proof against all future resource growth.

Health and five-second resource observations span the session. Detailed provider
storage filled its 262,144-record memory capacity during the session; later detail
is unavailable even though aggregates continued. Within the detailed prefix,
17 ready source tickets and 16 retirements/owner cleanups were captured. Distinct
owned tickets, not native pointer equality, supplied lifecycle identity.
No absence-of-races or exhaustive native scheduling proof is inferred.

No orderly detailed-writer footer was recorded. Archival used verified NMS exit,
collector exit, released file handles and identical inventories 30 seconds apart.
No detached native writer join or universal callback draining is claimed.
Archives were hashed and the original installation restored; normal game-managed
save/settings changes were retained. Original music and historical artifacts remained
unchanged. One older telemetry fixture remains an explicitly preserved historical
exception described in SUPERMOON_V1_KNOWN_LIMITATIONS.md, not a reconstructed original.

For that historical pair, intermittent pause-menu persistence and one unconfirmed
fuzzy-vocal report were open observations. Version 1.0.3 addresses menu persistence;
the historical vocal observation is not established as a decoder bug.
No private music names, raw logs, settings, saves or audit archives are included here.

Historical hashes belong to the corresponding sealed release, not the current
candidate recipe/checksums.
The public preparation only adds copyright comments/documentation and packaging
tools; it does not rebuild, install or load the DLLs. Production token equivalence
and exact-file privacy checks are recorded in docs/PACKAGING_VERIFICATION.json.
