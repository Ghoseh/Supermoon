# Supermoon v1.1.0

**Ready with Caveats for the limited desktop scope.** The owner explicitly accepted
the unresolved M2 and H1/U1 risks; neither finding is fixed. This v1.1.0 package
uses a separate public installation/recovery policy and retains the exact DLLs
tested as **1.0.4-rc1-private** in Phase 6. Existing RC records stay with their
matching RC tools. Local release artifacts are prepared for final owner review;
no public upload or release creation is authorized by this package.

Play your own supported WAV/MP3 files through the personal ByteBeat player's
Supermoon track in No Man's Sky. Supermoon prepares a disk-backed playlist,
normalizes loudness, shows filename titles and provides shuffle/navigation keys.
Output is 24 kHz mono; stereo inputs are downmixed. No playlist music is supplied.

## Requirements and compatibility

- Windows x64, ordinary desktop NMS, **Steam build 26385139 only**.
  `NMS.exe`: 89,535,560 bytes, SHA-256
  `a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.
  A general game version name is insufficient; unknown executable/native identities refuse.
- Python 3.10+ for local audio-bank generation; no extra Python packages.
- Enable mods and the generated Supermoon audio entry in NMS's normal mod settings.
  **No Steam launch arguments are required.** Setup never fabricates or edits settings.
- Unrelated mods can remain enabled. A competing `Binaries/winmm.dll` conflicts;
  proxy chaining is not supplied. Incompatible `audio/windows/init.bnk` overrides
  are outside support. Known enabled loose conflicts refuse; opaque packages and
  the effective winning native resource are not comprehensively inspected.
- VR is unsupported and not reliably detected. GOG/Game Pass, other executable
  builds, Vortex/MO2/other mod-manager deployment, and multiplayer compatibility
  are unqualified. Historical testing used Windows 11; this pair has no live OS matrix.

## Setup and use

Follow [INSTALL.md](INSTALL.md) for clean install, staged updates, durable backups,
status, recovery and uninstall. Keep NMS closed while changing files. Both DLLs,
the activation marker and a locally generated/enabled bank are required. The bank
is generated from your own game; native/generated NMS banks are never distributed.
Preserve `playlist.ini`, `Music`, `Cache`, `State` and unrelated files.
For the installed private RC, keep its matching tools and recovery records intact.
Do not use a differently versioned tool on that record. See the approved
[v1.1.0 transition](docs/V1_1_0_VERSION_TRANSITION.md) before any policy change.

Put supported music in `Binaries/Supermoon/Music`, or retain your existing INI path.
Only top-level files are scanned, at startup. Cold preparation can take time;
exit/relaunch after library changes. Start the personal ByteBeat Supermoon track.
Travel/reload may stop native playback; press Play manually when that happens.
If native playback continues, custom playback may continue. No automatic resume exists.

Default keys all require Ctrl+Shift: F6 previous/restart, F7 next, F8 toggle shuffle,
F9 one-shot shuffle-and-advance. Focus/overlay restrictions apply. Two embedded
owner-created shuffle cues are notification assets, not selectable songs.

## Audio behavior and unresolved limits

The Discoveries workaround changes the bank's Frontend ByteBeat volume so menus
can retain ByteBeat audio. It affects native ByteBeat tracks too and remains active
independently of the DLL. Disable/remove the verified override as well as proxy
activation when removing Supermoon or after an incompatible game update.

Soundtrack suppression follows validated custom playback intent through quiet/silent
passages and internal playlist EOF/refill. The exact DLL pair passed the controlled
owner-run desktop checks in two sessions, including normal Stop restoration,
Discoveries/pause cycles, Next, natural transitions, travel/reload manual recovery,
fresh launch and normal exits. See [qualification](docs/QUALIFICATION.md) for scope.

- **M2, Medium, accepted unresolved limitation:** native soundtrack volume may fail
  to restore after Stop under certain engine callback conditions, including no
  later qualified release opportunity or retry. Three original tests still fail.
  **Restoration after every Stop is not guaranteed.** No M2 gameplay failure has
  been confirmed in the reviewed evidence; likelihood and duration are not established.
- **H1/U1, accepted unresolved conditional limitation:** cleanup self-deadlock is
  reproduced in isolated tests; actual NMS gameplay reachability is unproven.
  A1's original High conditional severity remains. No gameplay occurrence has been
  confirmed; both live sessions exited normally. These results do not prove safety.

The audit's historical M2 release-blocker and H1 qualification-hold classifications
remain preserved, alongside the original failed tests. Current risk acceptance
changes the release disposition, not the technical findings or test outcomes.

Lifecycle stores remain finite: 4,096 Sources, 8,192 Parameters, 4,096 Events,
cumulative per process without reuse. Diagnostic counters do not establish live
session headroom. Manual Play recovery, brief restart/Next refill gaps, bounded
diagnostics and unsupported hot unload/universal shutdown remain limitations.

## Documentation

- [Formats](docs/SUPPORTED_FORMATS.md), [configuration/layout](docs/CONFIGURATION_AND_LAYOUT.md),
  [troubleshooting](docs/TROUBLESHOOTING.md), [known limitations](docs/SUPERMOON_V1_KNOWN_LIMITATIONS.md).
- [Candidate qualification](docs/QUALIFICATION.md), [migration/status](docs/MIGRATION_STATUS.md),
  [Phase 5](docs/REMEDIATION_PHASE5.md), [historical findings](docs/ACCEPTED_AUDIT_FINDINGS.md).
- [Release notes](docs/RELEASE_NOTES.md), [offline Nexus description](docs/NEXUS_DESCRIPTION.md),
  [version and recovery policy](docs/V1_1_0_VERSION_TRANSITION.md).
- [Build/provenance](docs/BUILD_AND_PROVENANCE.md), [developer guide](docs/DEVELOPER_GUIDE.md),
  [tests](tests/README.md), [dependencies](docs/DEPENDENCIES.md), [changelog](CHANGELOG.md).

## Credits and license

Original code/docs: MIT, copyright 2026 Supermoon contributors. Third-party terms
remain in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and `licenses/`.
The owner's notification recordings have separate terms in [ASSET_NOTICE.md](ASSET_NOTICE.md).
Development, reverse-engineering support, testing and documentation used OpenAI
Codex AI assistance. The original packaging pass used GPT-6; a complete historical
model roster is not established. The owner directed development and performed
historical and Phase 6 gameplay acceptance. No AI-generated recording claim is made.

No Man's Sky and ByteBeat belong to their rights holders. This independent mod
has no affiliation or endorsement implied.
