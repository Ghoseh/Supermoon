# Developer map and test boundaries

Build requirements/commands are in BUILD_AND_PROVENANCE.md. Ordinary users use
Setup.cmd without Python. All native source, ABI, playback intent, lifecycle,
synchronization and bank semantics remain those of the frozen live-tested pair.
supermoon_playback.cpp selects production defines/includes; winmm_bootstrap.cpp
is the proxy entry point. Source contains build-specific generated relocation/pin
metadata, not redistributed NMS executable/audio assets. Do not bypass identity gates.

Setup.ps1 is the PowerShell entry; Supermoon.Setup.psm1 manages four-file mutations,
backups/records/status and bank generation; Native.cs supplies bounded parsing and
Win32 file/process authority through built-in Framework compilation. No shell text
evaluation, downloaded helpers or persistent-policy change is part of setup.
The public version 1.2.0 differs from private recovery identity, not runtime bytes.
The only setup-script presentation change for public preparation is its banner.

Install ordering removes marker, replaces resources, then activates marker last.
Saved candidates/prior copies and immutable manifest precede selector publication.
Failures can be partial; recovery validates policy/path/identity/backups/current
state. It is not one transaction and does not solve hostile namespace races or
universal power-loss durability. Never force unknown resources or relabel records.

Native callback-visible clones, unwind, registry and playback state remain resident.
No hot unload/retry or guessed cleanup timeout is safe to infer after exposure.
Normal process exit, worker exit and universal callback draining are distinct.
M2/H1 remain unresolved; tests retain real failure/timeout assertions. No developer
should green those results by weakening assertions or adding expected-failure rules.
Private copied-game native fixtures and live diagnostics are deliberately omitted;
public regression sources use synthetic data or caller-provided private copies.
See tests/README.md and QUALIFICATION.md for actual scope.
