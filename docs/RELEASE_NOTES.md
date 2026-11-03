# Supermoon v1.1.0 — release notes

**Local release artifacts prepared; public upload/release approval pending.**
The owner selected v1.1.0 for the audio functionality beyond v1.0.2 and approved
Ready with Caveats for the limited desktop scope. This separate `1.1.0` public
package/recovery policy retains the exact DLLs qualified as `1.0.4-rc1-private`
in Phase 6. The installed RC record and matching tools remain unchanged.

## Audio features beyond v1.0.2

- Intent-driven native soundtrack suppression during custom playback, including
  quiet/silent passages and internal playlist endings/refill. Normal Stop return
  passed listening checks; M2 means restoration after every Stop is not guaranteed.
- Required locally generated Frontend bank workaround keeps ByteBeat audio through
  Discoveries/menu transitions. It also affects native ByteBeat tracks and persists
  independently of the DLLs, so removal must handle both components.
- Exact native integration for Steam build 26385139 and current indexed mod settings.

These capabilities were first delivered in v1.0.3 and are carried into v1.1.0 with
remediation and new qualification. They are not newly implemented by this documentation
pass. Playlist, supported WAV/MP3 preparation, normalization, filename display,
shuffle and keyboard controls are retained features from the earlier line.

## Reliability and installation improvements

- Recover suppression after demonstrated transient controller failures with newer
  validated playback intent; compensate audited stale-command ordering cases.
  Final callback cessation/failed-last-release conditions remain unresolved M2.
- Require specific enabled desktop bank registration, handle bounded first-time
  registration, and refuse known incompatible loose bank overrides without
  disabling unrelated mods. No required Steam launch arguments.
- Stage four-file installation with verified durable backups, version-bound recovery
  records, status/resume/qualified rollback/disable and unknown-file refusal.
  Ship an example INI only; preserve personal INI/Music/Cache/State/unrelated files.
- Block cache removal on incomplete playlist discovery and close the audited
  Init borrowed-read cleanup gap. Improve activation-stage/capacity diagnostics
  and safe build-output reservation. Finite capacity and native contracts remain.

## Accepted unresolved limitations

The project owner explicitly accepted these risks for the limited desktop release,
subject to final publication verification. Original findings, severity, evidence,
test assertions and historical audit release dispositions remain preserved.

- **M2, original Medium:** native soundtrack volume may fail to restore after Stop
  under certain engine callback conditions, including no later qualified release
  opportunity/retry. Three original regressions still fail. Normal restoration
  passed live tests; universal restoration is not guaranteed. No M2 gameplay
  failure is confirmed in reviewed evidence. Likelihood/duration are not established.
- **H1/U1:** isolated conditional cleanup self-deadlock remains reproducible; actual
  NMS gameplay reachability is unproven. A1's original High conditional severity,
  historical qualification hold and earlier U1 history remain. No gameplay occurrence
  is confirmed. Both live sessions exited normally; this does not prove exclusion.

Neither is fixed or proven harmless. M2's release-blocker classification originated
in the independent audit; current owner acceptance changes release disposition.
See [known limitations](SUPERMOON_V1_KNOWN_LIMITATIONS.md) for practical impact,
finite cumulative lifecycle stores/headroom, manual recovery/refill gaps, bounded
diagnostics, performance and unsupported hot unload/universal shutdown.

## Qualification and support

The Phase 6 owner-run checklist passed in two ordinary desktop Steam sessions:
native baseline; loud/quiet/owner-identified silent custom passages; normal Stop
return; repeated Play/Stop, Discoveries/pause, Next/EOF; manual travel/reload recovery;
fresh launch and normal exits. These are finite reported audible outcomes, separate
from SDK submissions, with no new four-hour endurance or all-schedule safety claim.

Final Phase 5 offline runs per normal/applicable UBSan mode: public 11 suites plus
595 checks; remediation **81 PASS / 3 M2 FAIL / 1 H1 TIMEOUT / 0 ERROR**; reliability
26 C++/6 Python PASS; installation 39 PASS. Both DLLs linked/reproduced, all 23
production units compiled, and private archives/source correspondence verified.
Python, C meter, assembly and native NMS are not UBSan-instrumented; H1 times out.
See [qualification](QUALIFICATION.md) for attribution and diagnostics.

Only ordinary desktop Windows x64 **Steam build 26385139** is supported:
`NMS.exe` 89,535,560 bytes, SHA-256
`a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.
VR is unsupported/not reliably detected. Other stores/builds, mod managers,
multiplayer and arbitrary packed-resource priorities are unqualified. Unrelated
mods are supported subject to foreign `winmm.dll` and incompatible `init.bnk`
conflicts. Python 3.10+ is required for setup; exercised Python was 3.12.

## Install, update and recovery

Use [INSTALL.md](../INSTALL.md), keep the game closed and other writers idle,
stage outside the game/TEMP, and retain matching trusted recovery tools/backups.
Generate the bank from your own supported game, enable it in normal NMS settings,
then use the personal ByteBeat Supermoon track. No native/generated game bank or
playlist music is distributed. Keep all required licenses/notices and the separate
terms for owner-created embedded notification cues.

The installed private RC remains on its matching record/tools; no record rewrite,
automatic migration, DLL rebuild or installed-file change occurred. See the
[v1.1.0 version policy](V1_1_0_VERSION_TRANSITION.md). Public installation/source ZIPs
have new hashes in the external release receipt and `SHA256SUMS`; old Phase 5 hashes
identify only the unchanged private archives.

New public setup checks: 39 installation/recovery cases PASS in each normal/
applicable UBSan mode and 17 disposable genuine-input CLI scenarios PASS for clean
install/update, rollback/resume/disable, interruptions and identity/version refusal.
Both ZIPs reproduce and verify source/member/manifest/link/notice/privacy closure.
No new game/runtime listening qualification is claimed; upload awaits explicit approval.
