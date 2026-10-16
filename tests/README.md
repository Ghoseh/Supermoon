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
The public wrapper was packaging-checked without rebuilding the runtime. See
docs/PACKAGING_VERIFICATION.json for which commands were executed in this pass.
