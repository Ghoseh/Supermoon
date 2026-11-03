# Supermoon v1.1.0 — offline Nexus Mods description

**Publication draft for owner review. Local artifacts are prepared and verified;
no Nexus upload or remote release has been created.**

Play your own supported WAV/MP3 music through the personal ByteBeat player's
Supermoon track in No Man's Sky. Supermoon prepares a disk-backed playlist,
normalizes loudness, displays filename titles and supplies navigation/shuffle keys.
Output is 24 kHz mono; stereo sources are downmixed. No playlist music is supplied.

## What's new beyond v1.0.2

- Native soundtrack suppression follows custom playback through quiet/silent
  passages and internal playlist transitions, with the Stop limitation below.
- A locally generated audio-bank workaround keeps ByteBeat audible through
  Discoveries/menu transitions, including native ByteBeat tracks.
- Remediated controller recovery, stricter required-bank admission, incomplete-scan
  cache preservation, Init cleanup, durable installation/recovery and diagnostics.

The audio features first appeared in v1.0.3. v1.1.0 carries them forward with
remediation and the exact DLL pair qualified in Phase 6; no new DLL rebuild is
planned. See [release notes](RELEASE_NOTES.md) for features versus reliability work.

## Read before downloading

**Ready with Caveats, limited desktop scope.** The project owner explicitly
accepted these unresolved risks, subject to final publication verification:

- **M2, original Medium:** native soundtrack volume may fail to restore after Stop
  under certain engine callback conditions. Normal restoration passed the live
  checks; every Stop is not guaranteed. Three original regressions still fail.
  No M2 gameplay failure is confirmed in the reviewed evidence; likelihood and
  duration are not established.
- **H1/U1:** a conditional cleanup deadlock remains reproducible in isolated tests.
  Actual NMS gameplay reachability is unproven; no gameplay occurrence is confirmed.
  A1's High conditional severity and historical audit classification remain.
  Both tested game sessions exited normally; this does not prove it impossible.

Neither issue is fixed or proven harmless. The audit's original release-blocker/
qualification-hold classifications and failed tests remain preserved. The owner
has chosen to accept the risks for this scope. Details: [known limitations](SUPERMOON_V1_KNOWN_LIMITATIONS.md).

Travel/reload may stop playback: press Play manually afterward. There is no
automatic resume. Brief restart/Next refill gaps remain. Lifecycle storage is
finite and cumulative per process without reuse; long-session headroom/native
exhaustion behavior is unqualified. No hot unload or universal shutdown guarantee.

## Requirements and compatibility

- Windows x64 ordinary desktop **Steam build 26385139 only**. `NMS.exe` must be
  89,535,560 bytes, SHA-256
  `a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.
  General game version names are insufficient; unknown native identities refuse.
- Python 3.10+ for setup and local bank generation; no extra packages. Setup was
  exercised with Python 3.12. **No additional Steam launch arguments.**
- Generate the required bank from your own supported NMS installation and enable
  mods plus the registered Supermoon audio entry in normal NMS desktop settings.
  Initial registration may require normal exit/restart after settings are saved.
- Unrelated mods may stay enabled. Another `Binaries/winmm.dll` conflicts; no proxy
  chaining is supplied. Incompatible `audio/windows/init.bnk` overrides are outside
  support. Known enabled loose conflicts refuse; packed/legacy priority and the
  native winning bank are not comprehensively inspected.
- VR is unsupported and not reliably detected. GOG/Game Pass, other builds,
  Vortex/MO2/other mod-manager deployment, multiplayer and a full OS matrix are
  unqualified. Manual staged installation is the supported workflow.

## Clean installation

Prepared downloads: `Supermoon-1.1.0-windows-x64.zip` (installation) and
`Supermoon-1.1.0-source.zip` (optional development source). Verify the accompanying
release `SHA256SUMS` through the owner's trusted release channel. Until upload is
approved, these are local artifacts, not an available Nexus download.

1. Close NMS fully. Keep Steam updates, extractors and other file writers idle.
2. Verify the downloaded archive against the final trusted published checksum,
   then extract to persistent staging **outside the game and TEMP**. Never merge
   the ZIP directly over `Binaries` or existing personal files.
3. Follow [INSTALL.md](../INSTALL.md): matching staged tool `status`, `install`,
   then `status`. Setup verifies identities, makes durable recovery records and
   installs exactly the paired DLLs, marker and locally generated bank. Foreign
   or unknown managed-file bytes are refused rather than overwritten.
4. Through NMS's normal settings, register/enable the bank. Add supported top-level
   music to `Binaries/Supermoon/Music`, or retain your existing INI-selected path.
   Only copy `playlist.example.ini` to `playlist.ini` if no personal INI exists.
5. Launch normally and select Supermoon in the personal ByteBeat player. Cold
   preparation can take time. Library changes require normal exit/relaunch.

Locations managed by setup:

```text
<NMS game>/Binaries/winmm.dll
<NMS game>/Binaries/bytebeat-playlist-experimental.enabled
<NMS game>/Binaries/Supermoon/supermoon_playlist.dll
<NMS game>/GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk
```

## Updating, recovery and removal

Keep personal `playlist.ini`, Music (including external paths), Cache, State,
diagnostics and unrelated files. Updates are staged separately; do not replace
whole directories or overwrite a live INI. Runtime managed-cache eviction after
a complete scan is separate from installation preservation and original music.

Retain verified durable backups and the exact version's trusted tool/recipe bundle.
Use matching `status` to inspect partial work, then deliberately choose `resume`,
qualified `rollback` or `disable`; these are alternatives, not a blind command list.
An interrupted operation may already have replaced the bank. Four-file atomicity,
arbitrary concurrent writer safety and tested power-loss durability are not promised.

Existing `1.0.4-rc1-private` users must keep their RC record and matching tools.
The v1.1.0 policy does not rewrite/import it. The safest immediate action
is to leave that byte-identical tested installation unchanged; any later migration
is separate and explicit. See [the version policy](V1_1_0_VERSION_TRANSITION.md).
Sealed v1.0.3 uses different DLLs/tools; do not mix versions or blindly extract it.

For removal or an incompatible game update, use matching verified `disable` with
NMS closed and retain personal data/records. Handle **both** proxy activation and
the loose bank: the bank's Frontend ByteBeat effect persists without the DLLs.
Do not force old pins to accept an updated game or delete unrelated mods/native banks.

## Controls and qualification

Defaults: Ctrl+Shift+F6 previous/restart; F7 Next; F8 shuffle toggle; F9 one-shot
shuffle-and-advance. Ctrl+Shift applies to every key. Focus/overlay limitations apply.
Supported format details and preparation/storage limits are in the accompanying docs.

Two owner-run Phase 6 desktop sessions passed the requested native baseline,
normal/quiet/owner-identified silent custom passages, Stop return, menu cycles,
Next/natural transitions, manual travel/reload recovery, fresh launch and normal
exits. These finite audible reports are separate from SDK command success; they
are not a new four-hour endurance test or universal engine-schedule proof.
The offline remediation suite still reports three M2 failures and one H1 timeout.
See [qualification](QUALIFICATION.md) and [troubleshooting](TROUBLESHOOTING.md).

## Credits and license

Original code/docs: MIT, copyright 2026 Supermoon contributors. Retain all supplied
third-party licenses/notices. Two owner-created embedded shuffle notification
recordings have separate terms in `ASSET_NOTICE.md`; they are not playlist songs.
No native/generated NMS bank or user music is distributed. Development, testing
and documentation used OpenAI Codex AI assistance under owner direction; the owner
performed live listening. No Man's Sky/ByteBeat belong to their rights holders;
no affiliation or endorsement is implied.

Report issues with reviewed, redacted minimal diagnostics. Logs may include personal
paths/song names. Do not upload music, caches, saves, credentials or private archives.
