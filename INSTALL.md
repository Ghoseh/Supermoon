# Install, update and remove Supermoon v1.1.0

Use this guide for the v1.1.0 setup tools. **Python 3.10 or newer is required for
installation, updates and recovery, but is not used during gameplay.** No extra Python packages
or Steam launch arguments are needed.

Only Windows x64 desktop NMS **Steam build 26385139** is supported. The supported
`NMS.exe` is 89,535,560 bytes, SHA-256:

`a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`

If your game has updated, do not bypass the compatibility check. See the release
page for support before installing. VR and other game builds/stores are unsupported.
Mod managers have not been verified; use manual staging.

## Before changing files

Close NMS completely. Keep Steam updates, other installers, extractors and file
restoration tools idle until setup is finished. Keep a durable backup outside
the game folder and temporary storage.

Preserve `playlist.ini`, Music, Cache, State, diagnostics and unrelated files,
including music stored outside the game. Keep any `.Supermoon-Recovery` folder
and the setup files that belong to its version. Never delete the whole Supermoon
or MODS directory to update or troubleshoot.

Supermoon manages only these four program files:

```text
<game>/Binaries/winmm.dll
<game>/Binaries/Supermoon/supermoon_playlist.dll
<game>/Binaries/bytebeat-playlist-experimental.enabled
<game>/GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk
```

The filenames above are required; do not rename them. Another mod's `winmm.dll`
or a conflicting audio-bank override must not be overwritten.

## Clean installation

1. Download `Supermoon-1.1.0-windows-x64.zip` and `SHA256SUMS` from the release page.
   Check the archive hash against that checksum file. The separately named source
   ZIP is for development and is not the installation package.
2. Extract into a persistent staging folder outside the game and TEMP. Do not
   merge the ZIP directly over `Binaries` or existing user files.
3. Check `python --version`. Use Python 3.10 or newer. If the `python` command is
   unavailable, install Python and make the command available before continuing.
4. With NMS closed, run the staged setup tool. Adjust both example paths below.

```powershell
$gameDir = 'D:\SteamLibrary\steamapps\common\No Man''s Sky'
$stageDir = 'D:\Mod Staging\Supermoon-1.1.0'
$setup = Join-Path $stageDir 'Supermoon\tools\install_supermoon.py'

python $setup status --game $gameDir
python $setup install --game $gameDir --package $stageDir
python $setup status --game $gameDir
```

`$gameDir` is the game root containing `Binaries` and `GAMEDATA`. `$stageDir`
contains `winmm.dll`, the enable marker and `Supermoon/`.

Setup verifies the supported game and matching files, generates the required
audio-bank override from your own game, and saves verified recovery copies before
replacement. It does not replace the original game audio archive. Native/generated
game banks are not supplied in the download.

A successful install reports `result: verified`; a subsequent status should report
`candidate_present` with a `verified` journal. Keep the recovery location and the
matching staging folder. These labels describe file state, not an audio listening test.

5. Launch NMS normally and use its normal mod settings to enable mods and the
   registered Supermoon audio entry. First-time registration may require a normal
   exit and restart after the game saves the entry. Do not hand-edit registration
   records or add Steam arguments to force activation.
6. Put supported songs directly in `Binaries/Supermoon/Music`, or retain the path
   in your existing INI. Select Supermoon in the personal ByteBeat player and Play.
   Preparation of a new library can take time.

The package includes an example INI. Copy it to `playlist.ini` only if no personal
INI exists. Defaults work without an INI: Music is the local `Music` folder and
the cache budget is 3072 MiB.

## Updating

Stage every update separately and retain personal files. For the same setup
version, check status, run `install` again and check status afterward. If a matching
operation was interrupted, use `resume` instead. The first prior-state backup is retained.

For a different version, keep its original setup tools and recovery records
together. New tools may refuse an older record even when some DLL bytes match.
Do not edit record versions or hashes to make them pass.

Before moving to a different version:

1. Verify and back up the existing four program files, recovery folder and matching
   setup files outside game/TEMP. Record which program files were absent.
2. If matching recovery tools exist, use them to inspect and disable the old
   installation. Verify the four program files are absent, then preserve and
   archive the complete old recovery folder outside the game before starting a
   new version's recovery folder. Keep verified copies; do not merge old/new records.
3. If the older installation has no matching recovery tools, identify its exact
   Supermoon files against that release's checksums before quarantining only those
   four files. Preserve the marker first, then DLLs and bank. If ownership or any
   identity is uncertain, stop and ask for help.
4. Follow clean installation from the new staging folder. Keep your personal INI,
   library and other files in place.

An installation made after removing the old program files records their absence.
Its rollback therefore returns to absence; it does not automatically restore the
previous mod release. Restoring an older release needs its full matching program
files, tools and supported game. Do not overwrite newer personal settings or State.

## Interrupted setup and recovery

Keep NMS closed. Run `status` with the matching tool and preserve all records.
`REFUSED_OR_INCOMPLETE` can mean some files, including the bank, were already replaced.
Do not assume a failed command left the installation untouched.

Choose one recovery action after checking the reported state:

- `resume` completes the saved installation using verified recovery copies.
- `rollback` restores the recorded prior state when that combination is supported.
- `disable` removes only recognized Supermoon program files and keeps personal data.

Example command, replacing the action deliberately:

```powershell
python $setup resume --game $gameDir
```

These actions are alternatives, not a sequence to run blindly. Unknown or corrupt
files must be preserved and investigated. Never change JSON/hashes or delete a
whole directory to force recovery. Keep records and tools outside the game before
uninstalling or moving NMS.

Setup uses a lock for cooperating tools, but cannot stop Steam or another program
from changing files afterward. The four-file operation is not one atomic change,
and recovery after power loss is not guaranteed. Keep other writers idle and
retain independent backups.

## Removal and game updates

With NMS closed, use the matching tool's `status`, then `disable`, then `status`.
This retains personal files and recovery copies. After an unsupported game update,
disable recognized files instead of forcing an older DLL pair to activate.

Remove both DLL activation and the recognized bank override. The bank changes
ByteBeat menu audio even when the DLLs are absent. Leave unrelated mods and native
game banks alone. Never replace DLLs or banks while the game is running.

See Known limitations and Troubleshooting before reporting unexpected audio.
