# Qualification summary for the exact shipped pair

Comprehensive independent review found no confirmed Critical or High blocker.
F1–F6 passed focused independent verification. Accepted C1 Medium and C2/C3 Low
remain described in ACCEPTED_AUDIT_FINDINGS.md, along with unverified U1–U3 contracts.
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

Intermittent pause-menu persistence and one unconfirmed fuzzy-vocal report remain
open observations. Neither is silently fixed or inflated into a proven decoder bug.
No private music names, raw logs, settings, saves or audit archives are included here.

The binary hashes are in build/recipe.json and RELEASE_DLL_SHA256SUMS.
The public preparation only adds copyright comments/documentation and packaging
tools; it does not rebuild, install or load the DLLs. Production token equivalence
and exact-file privacy checks are recorded in docs/PACKAGING_VERIFICATION.json.
