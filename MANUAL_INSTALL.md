# Manual installation — Supermoon v1.2.1

For Windows x64 desktop Steam NMS **build 26540391** only. This optional route
prepares files outside the game; you copy them yourself. **Setup.cmd remains the
automatic installer**, including verified v1.2.0 upgrades and recovery. The mod
DLLs are unchanged from published v1.2.1. This is manual-tools revision 1.

## Before you start

Close NMS. Keep Steam updates and other file writers idle. Verify the download
checksum, then extract the entire package to a persistent fixed local disk outside
the game and TEMP. Back up personal data outside the game.

Manual preparation requires a fresh inactive installation: all four activation
files listed below must be absent, and neither automatic recovery store may exist.
It rejects installed, partial and unknown states. If you have used automatic setup,
keep its records and use matching tools. **Do not delete records to pass this check.**
For published v1.2.0 use Setup.cmd > Upgrade.

Both workflows use built-in Windows PowerShell Desktop 5.1 and .NET; neither needs
Python. Manual preparation does not avoid Windows script/application-control
restrictions. If blocked, retain the error and read SETUP_SECURITY.txt. Do not
change antivirus or machine/user policy to force it.

## 1. Prepare files

Run **ManualSetup.cmd**, choose **1 Prepare**, enter the NMS root containing
Binaries and GAMEDATA, then enter a **new** output-folder path outside both the
game and extracted package. Its parent must already exist. Network, removable,
redirected and TEMP paths are unsupported.

Require `MANUAL_FILES_PREPARED`. The tool checks the exact executable, DLLs and
local archive, then generates the required audio bank from your own game files.
It does not install, enable, disable or write anything inside the selected game.
The prepared output contains:

- `COPY_TO_GAME`: the four activation files in the correct folders, plus a complete
  default INI if none existed during preparation.
- `DEFAULT_CONFIGURATION/playlist.ini`: all 12 settings, including +3 dB voice
  gain and Ctrl+Shift keyboard defaults.
- Instructions, licenses and `CONTENTS.json` with exact sizes and hashes.

If a personal INI exists, the copy tree omits it. Keep your file and edit settings
as needed using the default file as a reference. Manual preparation does not merge
settings or back up/change your existing configuration.

A `.supermoon-manual-INCOMPLETE-...` folder is a failed preparation. Do not copy
it. Retain its error for support; retry into a new output path after resolving the
cause. Never upload/share locally prepared folders: they contain bank data derived
from your game. The downloadable package contains no game bank.

## 2. Copy into the game

Keep NMS closed throughout. If an activation destination has appeared since
preparation, stop and resolve ownership; do not click Replace. Copy from
`COPY_TO_GAME` to the NMS root in this order, keeping the relative folders:

1. `Binaries/Supermoon/supermoon_playlist.dll`.
2. `GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk`.
3. `Binaries/winmm.dll`.
4. `Binaries/Supermoon/playlist.ini`, **only if still absent**. Otherwise keep your
   personal file. Never overwrite it with the default.
5. **Last:** `Binaries/bytebeat-playlist-experimental.enabled`.

Create `Binaries/Supermoon/Music` if needed; add WAV/MP3 tracks directly there.
Copy only these game-relative resources. Keep instructions, licenses, manifest
and the full prepared folder separately, outside the game.

Run ManualSetup.cmd, choose **2 Verify**, and select the same game and prepared
folder. Require `VERIFIED_MANUAL_INSTALL` before launching. This checks all four
activation identities, the executable and loose-bank conflicts; it does not prove
all mod interactions or audible output. Enable mods and Supermoon through normal
NMS settings. First registration can require a normal restart. Select Supermoon
in personal ByteBeat and Play.

Edit `Binaries/Supermoon/playlist.ini`; restart NMS after edits. VoiceGainDb accepts
0.0–6.0; 0.0 is neutral. Native personal ByteBeat may inherit gain. See README for
controls and limitations.

## Interrupted copying, disable and uninstall

Do not launch with a partial copy. Close NMS. Choose **3 Check before manual
removal** with the retained prepared folder. Require
`MANUAL_REMOVAL_CHECK_PASSED`. Missing files are allowed; unknown or modified
activation files refuse. The check deletes nothing. A changed game executable
does not prevent checking recognized files for removal.

Remove only the recognized files that remain, in this order:

1. `Binaries/bytebeat-playlist-experimental.enabled`.
2. `Binaries/winmm.dll`.
3. `Binaries/Supermoon/supermoon_playlist.dll`.
4. `GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk`.

Re-run the removal check; require `present: 0`. Leave Music, playlist.ini, Cache,
State, logs, settings, other mods and recovery records untouched. Do not delete
the entire Supermoon directory. The rollback target is absence, never another
release's DLLs. After removal, prepare again if you want a fresh manual copy.

Manual copying creates **no automatic transaction/recovery records**. Do not use
automatic Resume/Rollback to undo it. To switch to automatic setup, first complete
manual removal and verify all four files absent, then use Setup.cmd Install.
Automatic setup preserves the INI and fills missing defaults with a backup. Do
not run automatic Install over active manually copied files. Future upgrades from
a manual installation need the relevant release's documented transition procedure.

## Updating a manual installation

**Manual updates currently require removal first; in-place updates are not
supported.** Do not copy a newer release over the active manual installation.

1. Close NMS. Use the existing version's retained package/prepared folder to run
   the removal check, then remove only its four recognized activation files in
   the order above. Re-run the check and require `present: 0`.
2. **Keep Music, playlist.ini, Cache, State and logs. Do not delete the Supermoon
   folder or recovery records.** Removing the mod files does not require deleting
   your library or prepared cache.
3. Check the newer release's game compatibility and update instructions. Install
   its files using that release's supported manual procedure, or choose its
   automatic installer if it supports this inactive starting state. Preserve your
   existing personal INI instead of replacing it with the new example/default.
4. Verify the new installation before launching NMS.

To switch this v1.2.1 manual installation to automatic setup, complete removal
and verify all four files absent first, then run Setup.cmd > Install. Future
updates follow the relevant release's supported upgrade procedure. An automatic
v1.2.0 installation still uses its verified Upgrade route; do not manually remove
its files or discard its recovery records to follow these manual-update steps.

## Command line

Run the extracted `Supermoon/tools/ManualSetup.ps1` with built-in Windows
PowerShell and these named arguments:

```text
-Action prepare -Game "<absolute NMS root>" -Output "<new prepared folder>"
-Action verify -Game "<absolute NMS root>" -Output "<existing prepared folder>"
-Action removal-check -Game "<absolute NMS root>" -Output "<existing prepared folder>"
```

`-Package` optionally specifies the extracted ZIP root. Use the Windows ZIP;
the source ZIP lacks the required DLL/default-INI staging layout. Success returns
exit 0 and JSON; refusal returns exit 1. Check the result as well as the exit code:
`VERIFIED_INACTIVE` means no activation files present, not ready for playback.
