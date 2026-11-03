# Current validation scope — v1.1.0

The retained Phase 6 pair, first qualified as 1.0.4-rc1-private, uses public, remediation, reliability and installation
wrappers normally and with applicable UBSan. Original M2 cases still fail and H1
still times out; their tests/conditions remain unchanged and the remediation runner
must return failure. No expected-failure conversion is permitted. The owner
accepted M2 and H1/U1 as unresolved known limitations for the limited desktop
release; this does not change their severity, assertions or failure status.
Phase 6 live listening is separate finite evidence for the exact retained pair;
passing gameplay does not prove M2/H1 impossible. See [qualification](../docs/QUALIFICATION.md).
The private Phase 5 report retains exact commands, input hashes and raw receipts.
The installation ZIP includes this guide; running suites requires the source ZIP.

# Public regression subset

Use Windows x64, Python 3.10+ and the pinned Zig 0.15.2 toolchain from the build
guide. Run `python -B tools/run_tests.py --zig <zig.exe>` from the repository root.
The runner creates a fresh OS-temporary directory outside this checkout and uses
it as every test's working directory. Do not run old test executables directly in
a historical project/game directory. No test is passed a real music, cache, game
settings or diagnostic path. Existing relative fixture paths are contained by the
fresh disposable working directory; no output is copied back into this repository.

The selected original suites create synthetic WAV/PCM data and exercise startup
policy, queues, normalization/WAV parsing, cache, notifications, keyboard, forwarding
and compatibility. The final F1 corpus additionally generates 585 owned cases and
checks the compiled parser at guard pages with zero parser allocations.
`--ubsan` adds undefined-behavior instrumentation to these test builds; it does
not instrument copied game instructions. Tests do not launch NMS. The default
selection does not load the release DLLs.

WINMM tests default to own-process forwarding/ABI/failure checks. The separate
wrong-host DLL-load phase requires `--proxy <qualified-winmm.dll>` and copies that
verified file into the fresh output directory. It is skipped by default; no proxy
is built automatically. `--suite <name>` selects a suite without rerunning others.

This is a useful public subset, not the entire historical private qualification
suite. MP3 known-good licensed/private fixtures, actual-game copied-function tests,
live audits and raw diagnostic archives are not redistributed. The full retained
source includes legacy test/support mains which are not production DLL entry points;
the safe wrapper selects its inputs explicitly.
The current candidate was rebuilt and the public wrapper executed. See
../docs/PACKAGING_VERIFICATION.json for the scoped v1.1.0 summary. The original
Phase 5 summary is retained as ../docs/V1_0_4_RC1_PRIVATE_PACKAGING_VERIFICATION.json.
Public setup/package checks do not rerun or change the original runtime failures.

## Remediation regressions (Phase 0 baseline, extended through Phase 2A)

Current Phase 2A results and registration/conflict coverage are recorded below;
Phase 0/Phase 1 counts describe their original checkpoints.

The separate supported runner includes the production controller, actual adapter
and native wrappers, and required-entry policy assertions:

```powershell
python -B tools/run_remediation_tests.py --zig <zig.exe>
python -B tools/run_remediation_tests.py --zig <zig.exe> --ubsan
```

These commands intentionally return **1 on unchanged v1.0.3**. Every case asserts
desired behavior; there is no expected-failure whitelist or green assertion that
enshrines a defect. All cases run even after an assertion failure. Compile errors
are separate failures and retain compiler stderr. Each command prints a fresh
disposable output path containing per-case argv/cwd/exit/stdout/stderr receipts,
input hashes and `SUMMARY.json`. Keep those receipts in durable private evidence
before cleaning TEMP. No output or private audit evidence belongs in a release.

Select suites with repeated `--suite <name>` arguments:

| Suite | Actual implementation covered |
| --- | --- |
| `music_control_tests` | `MusicControl`/`PlaybackIntent`, failed SDK calls, recovery, reentrant and event-latched concurrent Stop, retirement/native replacement, stale observations, terminal identity, compensation supersession |
| `music_control_recovery_tests` | Additional Phase 1 schedules: exact-generation failure invalidation, newer commits during failed commands, first usable update, initialization loss inside setter/compensation, liveness recovery, failed compensation retry, bounded churn |
| `adapter_release_tests` | Included unchanged adapter, Core, real synthetic-input Engine, control-unlock assembly and recovery Term wrapper; nominal release, final-callback cessation, failed final release, digital silence and internal EOF/refill |
| `native_recovery_wrapper_tests` | Linked `recovery_native.cpp`: admitted Execute, exception cleanup, owner cleanup, retired/late callbacks and conditional self-wait |
| `dependency_policy_tests` | Actual MXML parser, enabled/global-disabled/malformed/disabled/missing required-entry settings; desired dependency assertions supplement its existing global-only contract |
| `required_audio_policy_tests` | Targeted desktop policy: duplicates/aliases, malformed/missing/mode-disabled entries, optional VR/index metadata, unrelated mods, limits/guard pages, registration confirmations |
| `audio_dependency_preflight_tests` | Owned files: ordinary desktop read-only preflight, first registration, active writer, targeted loose-bank collisions, identical bytes, unrelated mods and explicit package-inspection limits |

The native-wrapper suite is distinct from existing `recovery_tests.cpp`, which
tests compatibility/attempt records. The owned self-wait child has a three-second
deadline and is killed by its exact process handle and reaped; its `TIMEOUT`
counts as a failed liveness assertion. NMS reachability of that schedule remains
unproved. Only this synthetic child is terminated.

The SDK stub records the last **successful applied parameter**, independently of
the current logical intent and the last attempted request. A rejected command
does not update the applied value. Setter hooks deliberately finish applying an
old command after a newer Stop/retirement/commit; neither return values nor call
counts alone can satisfy the audio assertions. The compensation test injects a
new custom generation during neutral application and verifies it survives.

The final-callback cases stop the event schedule where provider/control traffic
ceases. They assert the release obligation without appending an arbitrary repair
pump. The original independent-release criterion remains unmet and the assertions
stay failing. The owner's risk acceptance defers further implementation; it does
not add a control opportunity or authorize SDK work in the audio observer.
Separate passing cases document repair when a later real unlock exists.

Adapter fixture setup represents an already admitted synthetic session. It uses
owned component bytes and source/event records, the real adapter callbacks and
unlock bridge, an owned synthetic PCM Engine, and stub SDK/native originals.
Activation, DLL publication/loading, game instructions and real game files never
run. Synthetic setup evidence is not a qualification of identity gates, actual
native lock ordering, Wwise application, audible behavior or all asynchronous
unwind locations. The EOF test consumes actual Engine frames; it does not execute
the game's native completed-frame loop. The controller-only quiet case has no
PCM input and is intentionally a narrower intent-persistence check.

The dependency suite does not read a real settings/bank file, select VR support,
simulate a winning resource override, or weaken a production pin. Missing/wrong
bank identity gates remain unchanged; effective load/mode/conflict qualification
belongs to Phase 2. C++ builds support UBSan; assembly and the separate C meter
build are not instrumented. Neither runner rebuilds a release DLL or launches NMS.

See [Phase 0 status](../docs/REMEDIATION_PHASE0.md) for baseline and coverage limits.

Phase 1 leaves the original test/assertion files unchanged. All 17 original
controller cases and eleven additional controller cases pass normally and under
UBSan. Complete remediation runs still return 1: three final-release cases and
two M3 policy cases fail, and H1's owned child times out. A final Stop without
provider/control callbacks still lacks a qualified release service. See
[Phase 1 partial status](../docs/REMEDIATION_PHASE1.md); no failed case is waived.

Phase 2A preserves `dependency_policy_tests.cpp` byte for byte and all five original
outcomes pass. The global parser corpus/startup tests bind to `global_mods_enabled`,
with every input, oracle and expected result unchanged. New suites add 42 cases;
all pass normally and under UBSan. Complete remediation selection reports 81 PASS,
three existing M2 FAIL and one existing H1 TIMEOUT per run, without filtering or
waiving failures. Registration-pending is never an enabled result. All file writes
in first-registration/conflict tests are synthetic fixture writes; production uses
read-only handles and requires all existing downstream identity gates.
See [Phase 2A](../docs/REMEDIATION_PHASE2A.md) for supported desktop behavior,
normal NMS registration evidence, first-install procedure and resource-load limits.
# Phase 3 installation/recovery fixtures

`python -B tools/run_installation_tests.py --zig <pinned-zig.exe> [--ubsan]`
runs deterministic Python setup/recovery tests in disposable directories. The
small owned process helper is compiled normally or with UBSan; Python is not
UBSan-instrumented. A helper named NMS.exe contains only fixture code, is never
the game executable, and is killed/reaped by its owning test. A benign tasklist.exe
sentinel must never execute. No game code or installed game/configuration writes occur.

Coverage includes personal-data preservation, example-only package ZIP merging,
exact identities, durable backups, interrupted commit/journal stages, retry,
rollback/disable, record/input corruption, real Windows sharing/locking, path
rechecks and process-enumeration failures. One deliberately noncooperating
after-check race characterizes the unsupported boundary: rechecks do not establish
arbitrary concurrent namespace safety. Original controller/adapter/H1 fixtures and
their failure statuses remain unchanged. See INSTALL.md for the supported contract.

## Phase 4 reliability fixtures

`python -B tools/run_reliability_tests.py --zig <pinned-zig.exe> [--ubsan]`
builds/runs 26 owned C++ cases and six Python output-reservation tests in fresh
disposable directories. Discovery/cache tests inject Win32 enumeration/metadata
errors and retain all reusable omitted pairs under quota pressure. Actual recovery
wrapper tests cover borrowed-read/query/ID/native-Init SEH cleanup, valid/failing
Init, exact factory/Clone/event exhaustion and ID reuse. Actual writer capacity
output and exported stage-reader/shared-formatting injections are exercised.
Stage injection is not real native publication; live headroom/failure schedules
remain unverified. Python, the existing C meter and assembly are not sanitized.
Original M2/H1 test sources/conditions are untouched and still fail/timeout honestly.
