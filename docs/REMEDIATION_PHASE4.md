> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Phase 4 reliability checkpoint

2026-11-02. Source-only remediation from `186d3ea`; no new runtime pair or release
qualification. All work/fixtures are isolated from installed NMS and user data.

| Finding | Disposition |
|---|---|
| A1-M5 / C1 | Fixed discovery/cache boundary: Win32 errors and explicit completion block all cache removal on incomplete scans. |
| A1-L1 / C2 | Fixed owned Init cleanup scope: borrowed reads after state mutation are finally-protected; exceptions propagate. |
| A1-I2 / C3 | Explicit cold activation stages and conservative exit/no-retry reporting; real native publication failure schedules remain unqualified. |
| A2-F5 | Safe TEMP ancestors can reserve separate sibling output; nested/protected/reparse paths still refuse. |
| A1-M4 / U3 | Capacity visibility improved; finite stores/limits/ownership unchanged and live headroom unmeasured. |
| A1-M2 / A1-H1 | Unresolved; original conditions and failing results remain visible. |
| A1-I1 / L2 | Deferred bridge/per-sample work; no speculative performance or calling-convention rewrite. |

Discovery uses explicit FindFirst/FindNext completion/error distinctions and
candidate metadata checks. Normal end or a checked empty directory can complete;
errors and 10,000-file truncation cannot. Sorting, top-level selection and cue
exclusion remain. Cache planning carries `discovery_complete`: incomplete scans
cannot remove obsolete, quota, orphan or invalid-pair files. Identified valid hits
can be reused; growth must fit without eviction. A later complete scan recovers
omitted pairs. Decode validity and enumeration completion are separate. Music is
never repaired/deleted. Concurrent directory edits are not an atomic snapshot.

Owned Source Init now enters its existing `__try/__finally` before the parameter
snapshot. Faults retire the source, restore previous TLS, release acquired leases
and clear executing, while preserving exception propagation and valid Init
behavior. The original malformed-caller fixture does not establish a gameplay
fault or universal recovery from invalid native memory.

Cumulative reserved Source/Parameter/Event slots and fixed limits, per-pool
reservation refusals, tracked aggregate refusals, ID reuse, late calls, overlap
and cleanup waits are sampled by the diagnostic writer at most once per second.
Counts are not songs/live voices or a coherent atomic transaction. Pool refusals
include capacity or bounded CAS contention; `refused_tracked` is not all host
refusals. Full/degraded samples include normal-exit/restart guidance. Existing log
byte/I/O limits still apply; failed startup or diagnostic saturation can prevent
visibility. No slots recycle or pool limits grow. No callback formatting/I/O is
added. Native handling of exhaustion and representative final-pair session births
remain unverified beyond the owned wrapper refusal tests.

Cold `AdapterActivationStage` reports preflight, possible lifecycle exposure,
core ready, publication started and published. Marks surround existing boundaries;
pointer-exchange order/residency remain. Bootstrap uses the observed stage rather
than guessing from result 10, including conservative repeated/concurrent refusal.
The export is diagnostic, not audible-success evidence or unload permission.
A future matching proxy/payload pair is required; current identity pins remain
unchanged and no new DLL pair was linked or made installable here.

`common.fresh` validates the actual reserved output directory, permitting safe
siblings under a TEMP ancestor while retaining source/protected/reparse refusal.
Labels cannot introduce paths. Arbitrary concurrent namespace safety is not claimed.

Validation: 26 new C++ cases PASS normally and under UBSan, plus six Python output
tests per run. Original A1 discovery/Init probes reproduce before the fix and,
unchanged, no longer reproduce afterward in both modes. Their post-fix exit 1 is
the old defect predicate becoming false; positive acceptance tests pass separately.
Actual source-wrapper TEMP-ancestor dry-run passes. All 23 production units compile
individually, without linking DLLs or rewriting pins. Complete public suites pass
in both modes (eleven suites, 595-check parser corpus, 25 receipts per run).
Original remediation suites retain 81 PASS, three M2 FAIL, one H1 TIMEOUT and zero
ERROR per mode. Unchanged Phase 3 protections retain all 39 passing installation
tests in both modes; Python itself is not UBSan-instrumented.

Activation tests inject cold stages into the actual exported reader/shared
formatter, not real native publication. The actual writer is exercised with owned
capacity/PCM data. No sanitizer diagnostics were observed on exercised paths;
unchanged C meter/assembly are not instrumented and H1 is a controlled timeout.
Listening, effective bank selection, native failure injection, live headroom and
performance remain unverified. No game launch, installed/configuration change,
shutdown hook, lifecycle redesign or release packaging/publication occurred.
Private fixture ZIPs belong only to the unchanged Phase 3 protection tests.

Evidence: `private evidence: PHASE4_REPORT.txt`.
Stop for owner review before packaging/final qualification. M2/H1 remain holds.
