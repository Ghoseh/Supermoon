# Version 1.0.2: NMS build 26295378

Status: built, reproduced and owner live-tested on 2026-11-01 (local time;
2026-11-02 UTC). The current warm session remains running. The owner authorized
publication of this hook compatibility update.

The behavior reference is Supermoon v1.0.0, commit `d5ff3ca`, under the existing
MIT, dependency and owner-created notification asset terms. This update starts
from published v1.0.1 commit `5ff0801e33485413494f41cf7860c21925c6008d`.

Target: Steam build 26295378, executable size 89,489,480 bytes, SHA-256
`d393c6782d29e1915854c0d5cf42505c8093740c05dc53705dbed7e0d1d4ac37`.

| Area | Adaptation | Performed verification |
| --- | --- | --- |
| Native copied functions | Address/relative-target profiles and hashes | 12 functions, 2,380 instruction contracts, 500 relative references |
| Bridges/unwind | Relocated sites; retained clone-local contracts | Seven bridges; 40 raw unwind records and 40 emitted associations |
| Allocator/source/parameter/owner lifecycle | Targets, cells and exact allocation CALL returns | Independent helper/table review; 90,929 native recovery checks; 120 native cycles |
| Hook publication | Updated cells and protection span | 45 injected failure boundaries, success and threaded paths passed |
| Filename/time/progress | Code pins, cells, globals and exact CALL returns | Independent 3,281-assertion mapping review plus 55 final source checks; all 15 full UI/time/register ranges covered |
| Startup | New executable/readiness/payload identities | Startup/compatibility/forwarding suites; exact executable accepted, five invalid/missing cases refused |
| Playlist/audio/cache/controls | Behavior retained | All 11 public suites and 585-case F1 guard-page corpus passed |
| UI integration | Behavior retained | Normal and UBSan UI/progress and writer coverage suites passed |
| DLL pair | Payload pin refreshed before proxy build | Second disposable portable build reproduced both DLL hashes |
| Live behavior | Exact candidate pair installed with backups | Custom playback, Next, UI and fast-travel recovery owner-confirmed; automatic EOF advancement and normal exit 0 logged; warm playback confirmed |

The five audio dispatch cells now span 0x850 bytes; the protection range was
updated accordingly. Runtime changes otherwise replace reviewed addresses and
identity pins. Clone-local offsets, inserted bridges, transformed unwind bytes,
allocation size, object fields and UI catalog mismatch guard remain intact.

Focused normal and UBSan UI runs each exercised 168,010 PCM calls through the
integrated preparation/Engine/UI path. Their reported progress checks were
360,755 and 360,763 respectively; these are synthetic own-process tests.

Private native instructions, fixtures, full executable snapshots and raw receipts
remain outside the repository. The previous full executable was not retained;
comparison uses hash-checked v1.0.1 copied-function fixtures plus the original
private image and retained prior review. Two previously changed-shape UI helpers
were corroborated by current call graph, complete ranges and prior review rather
than a full previous-image byte comparison. Runtime-initialized readiness values
and catalog contents are not established by static analysis.

Existing accepted findings and unverified contracts remain. Neither v1.0.1 live
acceptance nor the v1.0.0 four-hour endurance result qualifies this pair. No new
endurance or multiplayer compatibility claim is made.

Live evidence for this pair:

- Installed DLL hashes match the pair reproduced offline. Installation preserved
  Music, Cache, playlist.ini and game settings. The saved unsupported-build No
  entry for this exact executable was backed up and cleared after owner testing
  authorization; executable, native-site and payload checks remained enforced.
- The first session recorded 8,400,384 substituted frames, zero callback
  fault records, zero exceptions, zero stale-available frames and zero failed
  tracks. It exited normally with code 0.
- Two isolated 512-frame revision-empty blocks (1,024 frames total) occurred
  around navigation and travel rebinding. Maximum observed consecutive empty
  frames were 512. The captured natural EOF transition added no empty block.
  These finite counters are not an audible-gap measurement.
- The owner confirmed music again after Steam relaunch. The retained warm
  snapshot recorded 573,440 substituted frames, zero empty PCM, zero callback
  fault records, zero exceptions and zero failed tracks.
- Both preparations reused all 27 cached tracks and rebuilt none. Activation
  attempt-to-return spans were 0.596 seconds initially and 0.605 seconds on
  relaunch, excluding the wait for native audio registration. These observations
  are system/library-specific and are not startup performance guarantees.

No worker-drain, race-freedom, multiplayer certification or new endurance claim
is inferred. Raw logs and music names remain private. Historical v1.0.1 evidence
is in V1_0_1_MIGRATION_STATUS.md. The Discovery/pause-menu interruption and
background soundtrack overlap remain unresolved and are follow-up investigations,
separate from this compatibility release.
