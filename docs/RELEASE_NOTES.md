# Supermoon v1.1.0

Play your own supported WAV and MP3 music through the personal ByteBeat player,
with playlist navigation, shuffle, filename titles and loudness normalization.

## What changed

This release improves playback recovery, installation and troubleshooting:

- Better recovery from temporary soundtrack-control failures.
- Checks for the required enabled audio entry and known conflicting bank overrides.
- A staged installer with durable backups and status, resume, rollback and removal tools.
- Updates preserve personal configuration, Music, Cache, State and unrelated files.
- Interrupted music-folder scans preserve reusable cache entries.
- Clearer startup and per-session capacity messages.

The Discoveries/menu audio workaround and suppression of native music through
quiet custom passages were first released in v1.0.3. v1.1.0 retains those features
and adds the improvements above.

## Requirements

Windows x64 desktop NMS **Steam build 26385139 only**. Other game builds, stores
and VR are unsupported. Mod managers and multiplayer compatibility have not been
established.

Python 3.10+ is needed for installation, updates and recovery, not gameplay. No extra Python
packages or Steam launch arguments are required. Setup generates the required
audio-bank override from your own game files; enable it in normal NMS mod settings.

Unrelated mods may remain enabled. Another `winmm.dll` loader or an incompatible
`audio/windows/init.bnk` override conflicts with this mod.

## Known limitations

- The game's background soundtrack may remain quieter after Stop under some
  conditions. Normal restoration worked in listening checks, but every Stop is
  not guaranteed to restore it.
- A potential freeze during audio cleanup has been reproduced in isolated tests.
  Whether ordinary gameplay can trigger it remains unproven; no gameplay occurrence
  was confirmed in the reviewed tests.
- Travel or reload may stop playback. Press Play manually afterward.
- Brief restart/Next gaps remain. Playback is mono and the library is scanned at startup.
- Per-session audio storage is finite; a guaranteed session-length limit is not established.

Listening checks covered quiet/silent passages, Stop, menu transitions, Next,
automatic song changes, travel/reload recovery and normal exits. Those results
do not guarantee every possible game condition.

## Download and install

Use `Supermoon-1.1.0-windows-x64.zip` and check it against the attached `SHA256SUMS`.
The source ZIP is optional development material. Follow the installation guide:
extract into staging outside the game, keep NMS closed while changing files,
and preserve personal files plus matching recovery tools/backups.

The audio-bank menu change also affects built-in ByteBeat tracks. Removal must
handle both the DLLs and the override. No game bank or playlist music is distributed.
Retain the supplied code, third-party and notification-asset notices.
