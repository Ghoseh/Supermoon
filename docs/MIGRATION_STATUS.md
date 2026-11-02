# Version 1.0.3: NMS build 26385139

Status: qualified and prepared for the owner-authorized combined hook/audio release
on 2026-11-02. The exact owner-tested pair remains installed; NMS is closed.
This release starts from published v1.0.2 commit
`356f6d07cc9535d19577d34610018884a5526d44` and preserves the original MIT,
dependency and owner-created notification terms.

Target: Steam build 26385139, executable size 89,535,560 bytes, SHA-256
`a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.

| Area | Performed verification |
| --- | --- |
| Native profile | 12 copied functions; 2,452 instruction, 4,397 operand and 519 relative-field comparisons; 40 unwind records and seven bridge contracts |
| UI/time/register | Independent 3,777-instruction mapping, 258 contracts, 43 unwind records and 15 complete code pins; normal and UBSan integrated UI/writer tests |
| Recovery | 1,793-instruction mapping and 13 unwind records; 90,927 fixture checks, 120 cycles; 45 injected publication failure boundaries plus success/threaded paths; explicit bridge unwind checks |
| Identity | Exact current executable accepted; prior, changed, truncated, empty and missing executables refused |
| Public regression | All eleven suites and indexed-settings guard-page corpus; startup/parser UBSan tests passed |
| Music controller | 116 actual bridge checks, 24 adapter integration checks; 16,671 current-native SDK packet checks; 12 EOF and three shuffle streaming fixture cycles |
| Audio preparation | Native archive and bank unchanged by the update; locally derived output matches exact tested bank; reverse patch and negative-input checks; eight actual CLI derivation/install/conflict/identity cases |
| Build | Two portable builds reproduce both owner-tested hashes in build/recipe.json and RELEASE_DLL_SHA256SUMS |
| Owner acceptance | Native music before Play and after Stop, quiet-passage suppression, repeated Discoveries persistence, Next, travel and reload recovery passed |

The dispatch protection span is now 0x908 bytes. Native small-field layouts,
allocation contracts, clone-local bridges, transformed unwind and catalog guard
remain qualified; the current catalog fixture was independently checked.

The menu fix derives a ByteBeat Frontend-state volume change from the owner's
native bank. It affects native ByteBeat tracks in that state as well. The floor
fix adds a dedicated numeric parameter and copied Music-child curves. A fresh
accepted custom carrier enables suppression; sampled native enable/init state
can only clear owned intent. Controller calls occur after four existing control
unlocks, preserving the original unlock and register/unwind contracts. Provider
observation stays under its existing lock; no provider-thread SDK call or native
host-vtable hook is introduced. Stop/retirement sends the neutral setting. User
Music-volume settings and PCM silence are not modified.

The owner's approximately twelve-minute session recorded 33,890 provider/state
pairs, 13,029,425 consumed custom frames, two natural EOF
transitions and twelve accepted music-control commands across three owned source
incarnations. There were no exceptions, callback faults, failed tracks, stale
frames or dropped diagnostics. There were 20,943 empty frames during manual
restart/navigation transitions, at most 512 consecutive. The existing Engine
restart invalidates queued PCM before refill; this was reproduced without the
floor controller and remains a limitation. These counters do not measure audible
gap duration or establish all native schedules. An early snapshot labelled
before-play already contained custom playback; native-before-Play acceptance
comes from the owner's listening response, not that snapshot label.

No universal native draining, safe hot unload, race-freedom, multiplayer or new
four-hour endurance claim is made. Raw game instructions, fixtures, music names,
logs, saves, settings and private evidence remain outside the repository.
Historical v1.0.1 status is retained in V1_0_1_MIGRATION_STATUS.md. Version 1.0.2
qualification is preserved in its published tag; its unresolved menu/floor
observations are the issues addressed here, not current acceptance failures.
