# Supermoon v1.2.0

This update supports Windows x64 desktop Steam NMS build 26540391 and replaces the
fresh-user Python requirement with Setup.cmd and built-in Windows PowerShell 5.1.
No standalone installer EXE, external build tools, downloaded code or additional
third-party runtime is required by ordinary installation, update or recovery.

The exact DLL pair passed focused independent native review, offline setup/recovery
tests and one owner-run desktop session covering custom playback/ambience,
Stop/Play, playlist controls, travel/reload and restored playback. The pair is
byte-identical to the live-tested candidate, not rebuilt for this public package.
The public recovery policy is1.2.0, separate from private/older records even where
DLL bytes match. No automatic record conversion is provided.

Setup preserves personal Music, INI, Cache, State, settings, logs and unrelated
files. The download includes portable example configuration, an empty reference
Music folder and all notices; it distributes no native/generated bank or music.
The bank is derived from the user's verified installation. Existing old-build
RC/public DLLs must be disabled with matching legacy tools before migration;
those legacy tools may need Python. New users do not inherit that dependency.
Rollback after the documented migration means all-four absence, never restoration
of old incompatible RC DLLs. Follow INSTALL.md and VERSION_TRANSITION.md.

## Unresolved findings and observations

- M2 (original Medium): final_provider_stop_no_release_opportunity,
  last_source_retired_no_release_opportunity and
  failed_final_release_no_later_callback remain failing fixtures. Native music
  can remain quiet after Stop/retirement without a later safe control opportunity.
- H1/U1 (original High conditional): same_invocation_cleanup_liveness still times
  out in its owned fixture. If the schedule is reachable in NMS, a freeze may
  require termination. Actual live reachability remains unproved.
- Windows recorded RADAR_PRE_LEAK_64 for NMS during the focused session. The
  observation is unresolved; no cause or leak has been attributed to Supermoon.
  No crash or instability was reported by the owner. Sampled whole-NMS memory
  counters are not a measurement of Supermoon's allocations.

The retained remediation result is81 PASS,3 FAIL,1 TIMEOUT,0 ERROR (overall FAIL).
Owner acceptance and a successful finite session do not fix or reclassify these
findings. No four-hour endurance qualification applies to this pair. No safe hot
unload, universal native draining, all-schedule shutdown or maximum session length
is claimed. Manual Play after travel/reload, brief Next/refill gaps, mono output
and finite per-process storage remain. Read QUALIFICATION.md and known limitations.
