# Supermoon — your music, your journey

Give your next expedition its own soundtrack. Supermoon plays your supported
WAV and MP3 songs through No Man's Sky's personal ByteBeat player, with shuffle,
keyboard navigation and filename titles.

Bring your own music. Supermoon prepares a reusable disk cache, can normalize
loudness, and mixes stereo songs to mono for the game's playback path.

## Features

- Play a folder of songs with automatic playlist transitions.
- Keep native background music quiet during custom playback, including quiet
  and silent passages, with the Stop limitation below.
- Keep ByteBeat audible through Discoveries and menu transitions.
- Navigate, restart and shuffle using shortcuts.
- Use saved cache entries to reduce preparation on later launches.

## Before downloading

Supermoon v1.1.0 supports **Windows x64 desktop NMS on Steam build 26385139 only**.
VR and other stores/builds are unsupported. Mod-manager deployment and multiplayer
compatibility have not been established; use manual installation.

**Python 3.10+ is required for setup, updates and recovery. It is not used during
gameplay.** No extra Python packages or Steam launch arguments are needed.

The required audio-bank override is generated locally from your own game files.
You must enable it through NMS's normal mod settings. No playlist music or game
audio bank is included in the download.

## Getting started

1. Close NMS and keep Steam updates or other installers idle.
2. Download the Windows installation ZIP and verify its checksum. Extract it into
   a persistent staging folder outside the game and TEMP; do not merge it over Binaries.
3. Follow the installation guide to check status, install the matching files and
   generate the bank, then verify status. Keep the recovery folder and setup tools.
4. Enable mods and the registered Supermoon audio entry in NMS's normal settings.
   Initial registration can require a normal exit and restart.
5. Add supported songs directly to `Binaries/Supermoon/Music`, or keep your existing
   INI-selected folder. Choose Supermoon in the personal ByteBeat player and Play.

Preparing a new library can take time. Restart the game after adding or changing songs.

## Controls

| Action | Default shortcut |
| --- | --- |
| Previous / restart | Ctrl+Shift+F6 |
| Next | Ctrl+Shift+F7 |
| Toggle shuffle | Ctrl+Shift+F8 |
| Shuffle and advance once | Ctrl+Shift+F9 |

Overlays, focus and other bindings can affect shortcuts. Travel or loading a save
may stop playback; press Play manually afterward. There is no automatic resume.

## Compatibility and limitations

Unrelated mods can remain enabled. Another `Binaries/winmm.dll` loader or an
incompatible `audio/windows/init.bnk` override conflicts with Supermoon. Not every
packaged bank conflict can be detected automatically. Do not overwrite another
mod's files or change load order blindly.

The game's background soundtrack may remain quieter after Stop under some
conditions. Normal restoration worked in listening checks, but is not guaranteed
after every Stop. A potential freeze during audio cleanup has been reproduced in
isolated tests; whether ordinary gameplay can trigger it remains unproven. No
such gameplay occurrence was confirmed in the reviewed tests.

Brief restart/Next gaps remain. Output is mono; only the supported WAV/MP3 formats
are accepted. Per-session audio storage is limited, and no guaranteed maximum
session length has been established. Detailed troubleshooting is included.

## Updating or removing the mod

Keep your `playlist.ini`, Music, Cache, State, diagnostics and unrelated files.
Always stage updates separately and use the tools that match the installed version.
Keep verified backups and recovery records; do not edit those records or overwrite
an old setup with files from a different version.

The installer manages the paired DLLs, enable marker and required bank override.
For removal, close NMS and use the matching setup tool's disable command. It keeps
personal files and backups. Remove both DLL activation and the recognized bank:
the bank also changes built-in ByteBeat menu audio and remains active without the DLLs.
After an incompatible game update, disable the recognized mod files rather than
forcing an older version to activate. Do not delete unrelated mods or native banks.

## Source, downloads and help

- GitHub release and checksums: https://github.com/Ghoseh/Supermoon/releases/tag/v1.1.0
- Source: https://github.com/Ghoseh/Supermoon/tree/v1.1.0
- Installation guide: https://github.com/Ghoseh/Supermoon/blob/main/INSTALL.md
- Issues: https://github.com/Ghoseh/Supermoon/issues

When reporting a problem, include the game build and what happened. Review and
redact logs before sharing; they may contain personal paths or song names. Keep
music, caches, saves and full diagnostic archives private.

## Credits and permissions

Original code and documentation are MIT-licensed. Third-party licenses and notices
are included. The two embedded shuffle notification recordings have separate
asset terms; the code license does not grant standalone reuse of those recordings.

Development and documentation used OpenAI Codex AI assistance with human direction
and listening checks. No Man's Sky and ByteBeat belong to their rights holders.
Supermoon is an independent mod with no implied affiliation or endorsement.
