# Public regression sources - v1.2.0

Tests are developer-only. Never point mutating tests at a real game or personal
library. Use isolated disposable directories/private copies, NMS closed. Actual
game inputs and copied-native fixtures are not redistributed. End users run Setup.

From source root with Windows x64/Python 3.10+ and the pinned Zig from the build guide:

```powershell
python -B tools/run_tests.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
python -B tools/run_reliability_tests.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
python -B tools/run_installation_tests.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
python -B tools/run_remediation_tests.py --zig 'C:\Tools\zig-0.15.2\zig.exe'
```

Wrappers create fresh owned output and receipts outside source/game. --ubsan is
available where implemented. Remediation intentionally returns failure for the
three M2 cases and conditional H1 timeout; no expected-failure conversion applies.
Those findings are not rerun/fixed merely to package1.2.0. Private reviewed native
fixtures and broad game scheduling remain outside the public synthetic subset.

Python-free setup regression entry points (Windows PowerShell Desktop 5.1):

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File tests/powershell_setup_cases.ps1 -Work 'C:\Disposable\core48' -Module '.\tools\setup\Supermoon.Setup.psm1'
python -B tests/powershell_setup_integration.py --help
python -B tests/powershell_setup_remediation.py --help
python -B tests/package_installation_tests.py --dll-dir 'C:\VerifiedDLLs' --work 'C:\Disposable\package'
python -B tests/native_profile_tests.py --help
```

Integration needs private exact executable/archive inputs, extracted stage and a
benign owned process fixture; it never executes NMS. Python drives fixtures/oracles,
not the setup process. 48core/30integration/39original-installation coverage covers
bank identity, consent, recovery, interruptions, wrong inputs/policy, preservation,
sharing/path changes, concurrency and downloaded-script refusal. Source includes
all regression assertions; release receipts record actual final-package results.
See [Qualification](../docs/QUALIFICATION.md) for live/known limits, not universal safety.
