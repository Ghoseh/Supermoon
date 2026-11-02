# Version 1.0.3 qualification

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

The binary hashes are in build/recipe.json and RELEASE_DLL_SHA256SUMS.
The public preparation only adds copyright comments/documentation and packaging
tools; it does not rebuild, install or load the DLLs. Production token equivalence
and exact-file privacy checks are recorded in docs/PACKAGING_VERIFICATION.json.
