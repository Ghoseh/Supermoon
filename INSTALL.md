# Installation, update and removal

1. Exit NMS fully. In Steam, browse the game's local files and open Binaries,
   the directory containing NMS.exe. Verify the supported executable hash in
   docs/BUILD_AND_PROVENANCE.md. Do not bypass a mismatch after a game update.
2. Back up any existing winmm.dll, Supermoon folder and matching enable marker
   outside the game directory. Another mod's winmm.dll cannot simply be overwritten.
   Do not use the obsolete bytebeat-autostart.enabled one-song prototype marker.
3. For a clean installation, extract the installation ZIP into Binaries. It contains
   winmm.dll, bytebeat-playlist-experimental.enabled and Supermoon/. Keep the exact
   marker name/content despite its historical experimental label.
   For an update, merge deliberately: retain your existing playlist.ini, Music,
   Cache, State and unknown files instead of replacing them with defaults.
4. Add your own supported .wav/.mp3 files to Supermoon/Music (top level, no recursion).
   Both reserved shuffle cue names are excluded. No music is included in the ZIP.
   Cache and State are created automatically; default cache budget is 3072 MiB.
5. Generate the required audio override as described below while NMS is closed.
6. Enable mods using NMS's normal settings, then launch normally through Steam.
   Let cold preparation finish. Open the personal ByteBeat player, select Supermoon
   and press Play. See README.md for shortcuts and manual Play after native stops.

For step 5, Python
3.10+ is required for this one-time setup; no extra Python packages are needed.
From the game's `Binaries` directory, run:

```powershell
python Supermoon/tools/prepare_audio.py --game .. --install
```

The installation ZIP supplies `Supermoon/tools/prepare_audio.py`, its numeric
patch plan and `Supermoon/build/recipe.json`. The tool verifies the supported
executable, native archive and matching installed DLLs, then derives the override
from your own game files. It creates
`GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk`; the folder name
is retained for compatibility with existing installations. The native archive
is never changed. Keep or copy the printed backup/receipt directory out of
the temporary folder, especially when upgrading an existing audio override.
Unknown existing output is refused. Other mods replacing the same native bank
may conflict. Run the command again after installing a future supported update.

The portable example uses Music=Music relative to the mod installation. Settings
are sampled during startup, not continuously. Do not replace game settings or saves
with somebody else's files. Verify the release DLLs against SHA256SUMS before use.

## Disable, uninstall or roll back

Exit NMS completely. Move only this release's winmm.dll and
bytebeat-playlist-experimental.enabled out of Binaries to disable activation.
If an original proxy/marker was backed up, restore that exact original pair/state
only after verifying its identity; do not reactivate an obsolete prototype.
Retain personal Music, playlist.ini, Cache and State unless you explicitly want
their removal. Restore a backed-up Supermoon directory when rolling back an update.
Do not delete native game files, saves or unrelated mods. Live unload is unsupported.
After activation refusal, exit normally before attempting another launch.

Also move this release's generated audio override out of
`GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk` while NMS is closed;
otherwise its menu behavior remains enabled. Verify its SHA-256 is
`65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f` before removing
or replacing it. If the setup tool backed up a prior override, its printed
directory contains `receipt.json` and `prior-init.bnk`; restore that exact bank
with its matching prior DLL pair when rolling back. Retain personal music,
configuration, cache, state and normal saves/settings.
