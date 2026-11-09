# Install, update and disable Supermoon v1.2.0

Windows x64 desktop Steam NMS **26540391** only. NMS.exe must be 89,540,680 bytes,
SHA-256 `092236962ee050e419424699b5c343a0173f7773d5ad0866b7ec366d334c07d4`.
Other builds/stores and VR are unsupported. Never force an executable mismatch.

Fresh setup uses Windows PowerShell Desktop 5.1 and built-in .NET Framework support.
No Python, external compiler, audio converter, installer EXE or third-party runtime
installation is required. The source ZIP's Python/Zig tools are for developers,
not the ordinary installation path. Matching old recovery can still need Python.

## Before setup

Close NMS completely. Keep Steam updates, other installers and file writers idle.
Back up current INI, Music (external paths too), Cache, State, settings, logs and
matching recovery records outside game/TEMP. Preserve data; do not delete whole
Supermoon or MODS directories. Use persistent fixed local disks. UNC/device paths,
SUBST/network/removable mappings, reparse points and hard-linked managed files
refuse. Some unusual local storage also refuses conservatively.

Verify Supermoon-1.2.0-windows-x64.zip against the separately trusted release
SHA256SUMS. Extract completely into a durable stage outside the game and TEMP.
Do not run inside the ZIP, merge it over Binaries, or use the source ZIP as an
installation package. Keep the complete stage/recipe for future recovery.

## Fresh install

1. Double-click Setup.cmd. Confirm the detected Steam installation or enter the
   root containing Binaries and GAMEDATA, not Binaries alone.
2. Choose Install. Inspect the displayed target and type INSTALL to authorize it.
   Unknown files/conflicts, unsupported inputs and mismatched records refuse.
3. Require Completed: verified. Run Status afterward: require candidate_present,
   game_supported true and journal phase verified. File status is not audio proof.
4. Retain stage and recovery. Put supported WAV/MP3 files directly in the installed
   Binaries/Supermoon/Music folder, creating it if absent, or retain your INI path.
   No subfolder scanning. The ZIP includes an empty reference Music folder; setup
   does not copy songs or examples from staging into your library.
5. Launch normally and enable mods/Supermoon through normal settings. First-time
   registration may require ordinary exit/restart. Select Supermoon in personal
   ByteBeat and Play. Do not manufacture MXML records or add launch arguments.

Portable defaults use Music, Cache and State relative to the installed Supermoon
folder. No INI is required; default cache budget is 3072 MiB. The optional
playlist.example.ini must never overwrite an existing playlist.ini. See configuration.

Unsigned downloaded files may be blocked by RemoteSigned/Mark-of-the-Web,
SmartScreen or application/organization policy. Read ../SETUP_SECURITY.txt.
Verify provenance/hash before any deliberate individual Properties > Unblock on a
personally managed device. Never recursively unblock a folder, disable antivirus,
change persistent policy, use Bypass or elevate to evade restrictions. Managed
policy may require an administrator-approved workflow or prevent installation.
Setup does not download code, silently elevate, launch NMS or change persistent policy.

## Resources and preservation

Setup manages only:

```
<game>/Binaries/winmm.dll
<game>/Binaries/Supermoon/supermoon_playlist.dll
<game>/Binaries/bytebeat-playlist-experimental.enabled
<game>/GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk
```

It creates necessary parents, cooperative lock and .Supermoon-Recovery records.
The bank is generated from your verified native archive, never distributed; the
native archive is not replaced. Expected generated bank: 36,017 bytes, SHA-256
`65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f`.
Setup preserves personal Music, INI, Cache, State, settings, logs and unrelated mods.
Runtime separately updates Cache/State/settings/saves and replaces latest logs.
Archive logs before another launch; do not restore normal gameplay changes merely
to satisfy old preservation hashes. Opaque packed-bank conflicts are not fully visible.

## Updates and legacy migration

For the exact same recovery policy 1.2.0, run Status, Update, Status with matching
tools. The original prior-state backup is retained. Use Resume for interrupted work.
Keep personal data in place. Different policy versions cannot import/merge/relabel
records even when DLL bytes match. See [Version transition](docs/VERSION_TRANSITION.md).

Existing public 1.1.0/RC DLLs target the old executable and MUST NOT reactivate on
26540391. Use their matching legacy Disable, possibly Python, then verify all four
paths absent. Preserve verified original and disabled records/tools outside game/TEMP;
archive the complete old recovery root before making the new policy's root. Install
1.2.0 only after absence. Never rewrite selectors/manifests to force compatibility.
The live-tested private candidate has matching runtime bytes but a separate private
policy; its matching PowerShell tools must Disable it before any public-policy
transition. Release preparation does not require converting a working private install.

## Recovery and uninstall

Keep NMS closed after any failure. Some files may already have changed despite
REFUSED_OR_INCOMPLETE. Run matching Status and preserve all records. Choose one:

- Resume completes from verified saved candidate copies, with compatible executable.
- Rollback restores only the verified coherent recorded prior state. For the legacy
  migration above that state must be all absent, never old incompatible DLLs.
- Disable removes recognized candidate resources, including the bank, retaining
  personal data/records. It remains available for recognized installs after game
  updates even when activation/Resume/Rollback is blocked.

Each modifying action requires typed confirmation. To uninstall: Status, Disable,
Status, then verify all four paths actually absent. Retained records/empty directories
are expected. DLL absence alone is incomplete if the menu bank remains.
For sharing failures release the identified writer, inspect Status and deliberately
retry matching recovery. Missing/corrupt backups, unknown files or path/policy
mismatch require ownership-qualified review, never edited hashes, deleted records,
blind reinstall or old-DLL restoration. Keep NMS closed if fallback is incomplete.
See [Detailed inactive fallback](docs/NMS26540391_MIGRATION.md).
The operation is not one atomic transaction; cooperative locks do not exclude
Steam or hostile writers, and universal power-loss durability is unproved.

## Optional explicit CLI

From built-in Windows PowerShell 5.1, adjust these generic example paths:

```powershell
$stage = 'C:\Mods\Supermoon-1.2.0'
$game = 'C:\Games\No Man''s Sky'
$setup = Join-Path $stage 'Supermoon\tools\Setup.ps1'
& "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy RemoteSigned -File $setup -Action status -Game $game
```

Actions: status, generate, install, update, resume, rollback, disable. Install/Update
accept -Package $stage (default is extracted root). Modifying actions prompt;
-ConfirmAction INSTALL or the corresponding uppercase action is explicit automation
consent. Actions/options are case-insensitive, supplied as separate named values.
Stdout is one UTF-8 JSON object; diagnostics/prompts use stderr. Check exit code;
host/policy errors before script execution cannot produce script JSON. Generate
only derives in memory. -Record accepts a verified lowercase32-hex ID and bypasses
nothing. Pass CLI arguments to Setup.ps1, not the interactive Setup.cmd launcher.
