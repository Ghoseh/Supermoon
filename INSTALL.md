# Install, upgrade and remove Supermoon v1.2.1

For Windows x64 desktop Steam NMS **26540391** only. Supported NMS.exe is
89,540,680 bytes, SHA-256
`092236962ee050e419424699b5c343a0173f7773d5ad0866b7ec366d334c07d4`.
Do not force an executable or DLL mismatch.

## Before setup

Close NMS and keep Steam updates/other installers idle. Back up your personal
music, INI, Cache, State and recovery records outside the game. Verify the ZIP
checksum and extract all files to a persistent fixed local disk outside the
game/TEMP. Keep the entire extracted folder for recovery. No Python, compiler,
download or game launch is part of setup. Windows PowerShell Desktop 5.1 and
Windows/.NET Framework support are required. See `../SETUP_SECURITY.txt` for
script-policy restrictions; setup does not elevate or change persistent policy.

## Fresh installation

Run Setup.cmd, select the game root containing Binaries and GAMEDATA, choose
Install and type INSTALL. Require Completed: verified. Run Status: require
candidate_present, game_supported true and journal phase verified.

Add WAV/MP3 files directly to Binaries/Supermoon/Music, creating it if absent.
Enable mods/Supermoon through normal game settings, restart if registration
requires it, select Supermoon in personal ByteBeat, then Play.

## Upgrade published v1.2.0

Choose **8 Upgrade published v1.2.0 to v1.2.1** and type UPGRADE. Do not choose
ordinary Update for this version transition. The explicit procedure verifies
the exact published pair and all matching records, uses the included matching
v1.2.0 Disable logic, verifies inactive state, copies/verifies old records and
installs the new pair with separate v1.2.1 records. Personal files are not moved
or edited. Details and interruption recovery: [upgrade guide](docs/UPGRADE_1_2_0.md).
Private prototype, old RC, unknown and modified installations refuse.

## Configuration

Settings are read at startup from Binaries/Supermoon/playlist.ini. Setup creates
the complete file when absent, or backs up the original and adds only missing
settings. Existing values, comments and unknown settings remain intact. Backups
are kept under .Supermoon-Recovery-1.2.1/configuration. Missing VoiceGainDb
defaults to +3 dB; explicit 0..6 dB
values are preserved, invalid values are neutral. Set `[Audio] VoiceGainDb=0.0`
to opt out. Restart after edits. Defaults stay Ctrl+Shift+F6..F9; Ctrl-only and
Ctrl+Alt function-key bindings are optional under [Keyboard]. See README and
[configuration](docs/CONFIGURATION_AND_LAYOUT.md). Do not replace a personal INI
with playlist.example.ini. All 12 options are supplied automatically. Edit the
installed file after setup, not the integrity-pinned staging copy.

## Managed files and recovery

Setup manages these four activation resources:

```
Binaries/winmm.dll
Binaries/Supermoon/supermoon_playlist.dll
Binaries/bytebeat-playlist-experimental.enabled
GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk
```

Separately, setup creates/completes playlist.ini as user configuration. Disable
and rollback retain it and its original backup; they remove the four activation
resources only. Completion uses verified backups and atomic replacement. Sharing
failures can be retried with matching Resume/Update; ambiguous duplicate sections
or unsupported encodings refuse without changing the original INI.

The legacy marker/bank names are retained for runtime compatibility. The bank is
generated from verified local game input; no native archive is replaced.
v1.2.1 records live in `.Supermoon-Recovery-1.2.1`. Old `.Supermoon-Recovery`
records remain in place and are also copied into the new store's archives.
Keep both stores; do not edit selectors, manifests, hashes or backup files.

With NMS closed, Status then Disable removes the recognized four resources and
retains personal files/records. Verify all four absent afterward. Recognized Disable remains available after
an executable update even when installation/Resume are refused. Rollback uses
only its own verified prior state; after fresh setup or v1.2.0 Upgrade that state
is all absent. It does not reinstall v1.2.0 DLLs. For interruption use Status then
Resume, Upgrade again, or Disable as described in the upgrade guide. Unknown
files or corrupt backups refuse; keep NMS closed and preserve records for support.

Ordinary Update is for the same v1.2.1 identity policy. Retain original music and
records independently. File-by-file mutation is recoverable but not one atomic
transaction; universal power-loss durability is not claimed.

## Explicit PowerShell CLI

Run the extracted `Supermoon/tools/Setup.ps1` with `-Game` set to the absolute
game root, `-Package` set to the extracted ZIP root, and `-Action upgrade`.
Type UPGRADE, or use explicit automation consent `-ConfirmAction UPGRADE`.
Other actions: status, generate, install, update, resume, rollback, disable.
Pass named arguments to Setup.ps1, not Setup.cmd. Noninteractive output is JSON;
check its result and process exit code. Resume during a pre-install upgrade
needs the matching extracted package. Never use unrelated release tools/records.
