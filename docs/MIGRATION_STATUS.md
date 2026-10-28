# NMS build 26271400 compatibility update

The reference is Supermoon v1.0.0, commit d5ff3ca, MIT first-party source with the existing third-party and asset terms retained. This is a compatibility repair of the owner's mod, with no new gameplay features or public publication.

Target executable: Steam build 26271400, 89,491,528 bytes, SHA-256 `d1c7155764271eff0733c37d1849e685a11da3d54a41d685d7898fc0aad83a87`.

| Area | Treatment | Evidence |
| --- | --- | --- |
| Playlist, WAV/MP3, normalization, controls | Keep | Full public regression selection passed |
| Native provider/update/setter/pool clones | Adapt RVAs and relocation targets | 2,380 instruction contracts and 500 relative fields checked |
| Native bridges | Keep register/stack contracts | All seven sites statically checked; updated-native harness passed |
| Unwind | Keep clone-local transformations after verifying new native inputs | 40 native records and all chains checked |
| Allocator/source/parameter/owner lifecycle | Adapt cells, callers and native targets | 90,928 checks, 120 native cycles; 45 publication failure boundaries passed |
| Filename/time/progress | Adapt cells, globals, exact return sites and code pins | 40 static contract checks; normal and UBSan focused suites passed |
| Startup/profile/payload gates | Adapt exact identities | Startup/parser/forwarding/compatibility tests passed; exact new DLL pair pinned |
| Live playback and UI | Live acceptance passed | PID 5012 activated successfully; custom PCM, filenames and time replacements observed; owner confirmed Glokk40Spaz and used Next |

The native widget renderer includes reordered instructions. Exact title/time/row CALL returns and R15/R12/RBP, output/frame, widget role/parent, catalog and XMM6 contracts were checked separately. Changed native widget state byte offsets are not accessed by Supermoon. Auxiliary code pins cover whole corresponding new functions rather than obsolete unwind split positions.

Known deviations: no intended feature changes. This build supports only the new executable, and rejects the previous executable. The accepted v1.0.0 findings and unverified native contracts remain applicable; this repair does not claim to resolve them.

Offline results: full public selection passed, including the 585-case F1 guard-page corpus. Focused UI normal and UBSan runs each passed approximately 360,765 checks, including an integrated real preparation/Engine path with 168,010 PCM calls. All tests used disposable paths and synthetic or private native fixtures. No real game process was used by these tests.

Installation: verified backups of the pre-install mod tree, NMS profile and settings. The final retained 27-song library was copied into the active installation; all 27 music hashes verified. Existing retired folders and the old two-clip tree remain preserved. The game executable and settings were not modified.

The exact DLL pair was also reproduced with the portable build wrapper in a second disposable directory. First live activation returned zero after 116.877 seconds; preparation reused all 27 cache entries and rebuilt none. Startup latency is recorded, not treated as a warm-start performance guarantee.

Live PID 5012: owner confirmed custom Glokk40Spaz audio. Next advanced track/serial/revision from 9/1/1 to 22/2/2. No callback exception, callback fault, stale-available frame or failed track was recorded in initial captures. One 512-frame empty-PCM block was observed around navigation; further steady playback did not increase that counter through the after-Next capture. This is about 21.3 ms at 24 kHz, not a claim about what was audible.

Automatic EOF advancement is confirmed by the owner and by a completed-track increment with track 22/serial 2 advancing to track 10/serial 3 at the same queue revision. The empty-PCM counter remained at 512 through this transition; no additional empty block was observed.

The owner confirmed manual Play recovery after teleport/warp. Logs show source retirement and a later qualified commit, with custom PCM resuming on the recovered endpoint. PID 5012 then exited normally with code 0.

Warm PID 8460 activated in 0.651 seconds, reused all 27 cache entries, verified all five audio redirects and produced fresh custom PCM after a new native commit. The owner confirmed that music still works after relaunch. The repair is installed and live acceptance passed.

Final first-session diagnostics: 12,944,384 substituted frames; zero callback faults/exceptions, stale-available frames or failed tracks. Nine isolated 512-frame revision-empty blocks occurred around navigation/rebinding (4,608 frames total; maximum consecutive 512). No additional empty block was added by the captured automatic EOF transition. These are diagnostic observations, not an audible-gap measurement. The original pair's four-hour endurance qualification does not qualify this pair.
