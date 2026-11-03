# Supermoon v1.1.0

Play your own music in No Man's Sky through the personal ByteBeat player.
Supermoon supports WAV and MP3 playlists, loudness normalization, filename titles,
shuffle and keyboard navigation. Stereo sources are mixed to mono for playback.
No playlist music is included.

[Download v1.1.0](https://github.com/Ghoseh/Supermoon/releases/tag/v1.1.0)
and follow [Installation](INSTALL.md) before copying files into the game.

## Features

- Play a folder of supported WAV and MP3 songs, with automatic playlist transitions.
- Keep the game's background soundtrack quiet during custom playback, including
  quiet and silent passages. Soundtrack restoration after Stop has a limitation below.
- Keep ByteBeat audio audible through Discoveries and menu transitions.
- Show filename titles and use shortcuts for previous, next and shuffle.
- Prepare a reusable disk cache to reduce work on later launches.

## Requirements

- Windows x64, ordinary desktop No Man's Sky on Steam.
- **Steam build 26385139 only.** Other game builds are unsupported.
- **Python 3.10 or newer for installation, updates and recovery.** No extra Python
  packages are required. Python is not used during gameplay.
- Generate the required audio-bank override from your own game files, then enable
  it in NMS's normal mod settings. The setup tool handles generation.
- No additional Steam launch arguments are required.

VR and other stores are unsupported. Mod-manager deployment, multiplayer
compatibility and a full Windows-version matrix have not been established.
Use the documented manual installation method.

## Playing your music

Put supported songs in `Binaries/Supermoon/Music`, or keep your existing
`playlist.ini` music-folder setting. Only files directly inside that folder are
scanned. Restart the game after changing the library. Preparing a new library
can take time; later launches can reuse the cache.

Select Supermoon in the personal ByteBeat player and press Play. Travel or loading
a save may stop playback; press Play manually afterward. Automatic resume is not
provided.

Default shortcuts:

| Action | Shortcut |
| --- | --- |
| Previous / restart | Ctrl+Shift+F6 |
| Next | Ctrl+Shift+F7 |
| Toggle shuffle | Ctrl+Shift+F8 |
| Shuffle and advance once | Ctrl+Shift+F9 |

Focus, overlays and other key bindings can affect shortcuts.

## Compatibility and known limitations

Unrelated mods can remain enabled. Another `winmm.dll` loader or an incompatible
`audio/windows/init.bnk` override conflicts with Supermoon. The installer cannot
detect every packaged audio-bank conflict; do not overwrite another mod's files.

The game's soundtrack may remain quieter after custom playback stops under some
conditions. Normal restoration worked in listening checks, but it is not guaranteed
after every Stop. A potential freeze during audio cleanup has also been reproduced
in isolated tests; whether normal gameplay can trigger it is unproven. No such
gameplay occurrence was confirmed in the reviewed tests.

Brief restart/Next gaps, finite per-session audio storage and manual Play after
travel/reload remain limitations. No guaranteed maximum session length is established.
See [Known limitations](docs/SUPERMOON_V1_KNOWN_LIMITATIONS.md) and
[Troubleshooting](docs/TROUBLESHOOTING.md).

The menu audio change also affects built-in ByteBeat tracks and remains active
while its bank override is installed. Removal must handle both the DLLs and that
override. Keep personal music, configuration and backups when updating or removing
the mod.

## More information

[Installation and recovery](INSTALL.md) · [Audio formats](docs/SUPPORTED_FORMATS.md) ·
[Configuration](docs/CONFIGURATION_AND_LAYOUT.md) · [Changelog](CHANGELOG.md) ·
[Release notes](docs/RELEASE_NOTES.md)

Developer test and qualification details remain in the engineering documentation.

## Credits and permissions

Original code and documentation are MIT-licensed. Retain the supplied third-party
notices and licenses. The two embedded shuffle notification recordings have
separate terms in `ASSET_NOTICE.md`; the code license does not grant standalone
reuse of those recordings.

Development and documentation used OpenAI Codex AI assistance with human direction
and listening checks. No Man's Sky and ByteBeat belong to their rights holders;
Supermoon is an independent mod without implied affiliation or endorsement.
