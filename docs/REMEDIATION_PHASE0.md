> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Remediation Phase 0

Baseline: audited v1.0.3, commit
`032a56b70336036f5cd1a4a82498dbb670198437`, branch
`remediation/v1.0.3-phase0`. All 161 source files matched the sealed source before
test additions. All 214 sealed release files matched A1's original inventory.
Both ZIP manifests, ZIP CRCs/member sets and staged bytes passed verification.

| Baseline artifact | SHA-256 |
| --- | --- |
| Installation ZIP | `ec6273ff74e3aa81e1749fce61ae42ca9d815db7825bce23f5ff8113f73287bc` |
| Source ZIP | `423b2037d83b7c33d6a5cbda3e5e095a9bddb9390804e95c7fb25c668a83fa35` |
| winmm.dll | `6911d04b39ba97f49cfdd5e52dc36f34a7f2a1f7e6faa5aeed59e97ab7b6f02b` |
| supermoon_playlist.dll | `b51cb3c3b94c589c8ed9decaa758444dff260667a4e6a93e0fa91eebcc079d1d` |

Read-only checks also matched the pinned NMS executable, installed DLL pair and
36,017-byte generated bank. No game input was copied into tests or modified.
Compiler verification uses the existing hash-pinned Zig 0.15.2 executable.

The eleven existing public suites and final F1 corpus passed in normal and UBSan
builds, with 25 successful compile/run receipts per run. Each parser run reported
595 checks, zero failures, zero audited allocations and guard pages enabled.
The normal forwarding run included the already-qualified proxy's wrong-host
load test; the UBSan run omitted that optional DLL-load phase. These are offline
subset results, not a new audible/native lifetime qualification.

The additional runner is documented in [tests/README.md](../tests/README.md).
Assertions express the behavior required after remediation and return failure
against unchanged v1.0.3. No defect is implemented, fixed, waived or called an
expected passing result. The suites provide these handoff boundaries:

| Finding or protection | Regression evidence |
| --- | --- |
| A1-M1 | Active engine loss and recovery; failed enable/successful fallback; failed release/later repair; failed enable and fallback; fresh validated generations must recover while old intent stays invalid |
| A1-M2 ordering | Stop, retirement and a newer native commit inside setter; real latched concurrent Stop; new custom commit during neutral compensation; assert logical and applied state at outer return |
| A1-M2 release opportunity | Actual adapter final provider Stop, actual last-source Term and failed final neutral call with no future callback; separate nominal/later-unlock cases show when repair does happen |
| Identity and stale work | Older same-ticket and older-source observations cannot clear newer generations; retired tickets cannot restart; terminal invalid identity stays closed |
| Continuous carrier | Actual Engine plays silent/audible synthetic songs across internal EOF/refill without inventing native Stop; native Stop still releases on a real subsequent control unlock |
| A1-H1 focused fixture | Actual wrapper self-cleanup schedule has a bounded owned-child observation; timeout remains a conditional qualification failure, not proof of NMS reachability |
| Wrapper protections | Execute admission/TLS/exception cleanup, normal owner cleanup and late/retired callback rejection |
| A1-M3 policy | Separate actual-parser fixtures for enabled, global disable, malformed, explicitly disabled and missing required entries |

Both final full runs (normal and UBSan) returned **1**, with identical results:

| New suite | Passing cases | Failing cases | Controlled timeouts |
| --- | ---: | ---: | ---: |
| Music controller | 8 | 9 | 0 |
| Actual adapter/release | 3 | 3 | 0 |
| Native recovery wrappers | 3 | 0 | 1 |
| Dependency policy | 3 | 2 | 0 |
| Total per run | 17 | 14 | 1 |

All test builds succeeded. There were zero harness errors and no UBSan diagnostics
in the final runs. The self-wait child reached the owned cleanup call before its
three-second timeout and was killed/reaped. A1-M1 contributes four recovery
failures; A1-M2 ordering/compensation contributes five controller failures and
three release-opportunity failures. Two dependency assertions fail separately.
The old stale-observation, retired-ticket, terminal-identity and nominal-release
protections remain green. These counts describe regression evidence, not fixes.

The final-callback fixtures deliberately expose a missing service opportunity.
Phase 1 must establish that opportunity on a safe control path and exercise it
without continuous pumping or audio-thread SDK calls. The setter fixture models
successful synchronous application and rejected calls with no application; it
cannot establish actual native/Wwise queue ordering or audible restoration.
The compensation case must execute its newer-commit hook to pass, preventing a
vacuous pass if compensation never runs.

Original A1/A2/A3 reports, roadmap, fixture source and raw receipts were copied
byte for byte into a separate durable evidence directory, retaining attribution.
A3 scratch/private build caches are excluded and untouched. Initial harness
compiler errors and subsequent receipts are retained as development history,
separate from final assertion failures.

Production files, release archives, manifests and pins remain unchanged. The
baseline public manifests continue to describe the sealed release; this worktree
with added tests is not a newly sealed package. No DLL build, install, game launch,
user-configuration edit or publication occurred. The branch is a test checkpoint
for Phase 1 review, not a release candidate. Stop here until owner approval.
