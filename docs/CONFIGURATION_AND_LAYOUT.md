# Portable configuration and installation layout

This describes the exact audited and endurance-tested v1.0 runtime. `<game-executable-directory>` means the directory
containing NMS.exe, independent of the Steam library/drive or developer workspace.

```text
<game-executable-directory>/
  NMS.exe
  winmm.dll
  bytebeat-playlist-experimental.enabled
  SETTINGS/GCMODSETTINGS.MXML
  Supermoon/
    supermoon_playlist.dll
    Music/
    playlist.ini          (optional)
    Cache/                (managed preparation data)
    State/                (preferences and compatibility/attempt ownership)
```

The existing enable-marker filename is intentional runtime data, despite its
experimental label. Its required contents are exactly
`ByteBeat automatic startup v1` followed by CRLF, with no extra text. Startup also
requires a unique logical DisableAllMods=false in valid supported settings and
the exact executable/payload/profile gates. Do not rename markers, remove safety
checks or replace user settings to force activation. Late settings edits after the
final sample are not continuous cancellation; see the developer guide.

Music defaults to `Supermoon/Music`, resolved from the executable/mod location,
not the shell's working directory. Discovery reads top-level supported WAV/MP3
files, sorts paths and excludes the reserved shuffle on.wav/off.wav cue names.
See SUPPORTED_FORMATS.md for strict finalized RIFF/data header requirements;
Supermoon does not repair malformed files. Cold preparation may delay availability.
Accepted C1 means interrupted discovery can silently omit tracks and evict their
decoded cache entries; use a stable, preverified local library for endurance.
Subfolder recursion and runtime library refresh are not implemented. Do not bundle
private music. Respect the content owner's rights when preparing a public package.

Optional settings below state current defaults. They are documentation examples,
not replacement files to copy over existing configuration:

```ini
[Playlist]
Music=Music
CacheMiB=3072
[Normalization]
Enabled=1
TargetLUFS=-16
TruePeakDBTP=-1
[Keyboard]
Enabled=1
Previous=Ctrl+Shift+F6
Next=Ctrl+Shift+F7
ToggleShuffle=Ctrl+Shift+F8
ShuffleAndAdvance=Ctrl+Shift+F9
[Notifications]
ShuffleSounds=1
```

CacheMiB accepts 16..65536 and covers the reserved Cache root, including legacy and
unknown files and preparation scratch. Unknown/leased files are retained and can
prevent new entries. The native parser also accepts absolute Music overrides, but
portable examples use relative paths. Normalization values must be finite within
-60..0 LUFS and -20..0 dBTP. Disabled normalization retains peak headroom processing.
Invalid normalization/cache settings can refuse preparation. Invalid keyboard or
notification settings disable that optional subsystem and are logged; they do not
silently replace the user's chosen bindings. Keyboard shortcuts accept unique
Ctrl+Shift+F1..F24 or None, excluding F12; plain F6/F7/F8/F9 are not the defaults.
Shortcuts use foreground/capture/activity heuristics and do not consume native keys;
conflicts with game overlays/bindings are not guaranteed absent on every setup.

Native travel/reload may stop ByteBeat; press native Play manually to restart
custom playback through a qualified source. No automatic resume is implemented.
Shuffle mode persists separately under State; cross-session song position/history
does not. Cues are embedded, not additional music entries. Reserved old cue files
are excluded from discovery rather than treated as selectable tracks.

For installation, inspect and preserve an existing winmm.dll
before changing anything; do not assume another proxy can be replaced or chained.
Keep existing Music, playlist.ini, Cache, State, settings and unknown files. Do not
overwrite configuration with examples. Installation/removal must occur with NMS
fully exited; hot DLL unloading is unsupported. A removal/rollback plan should
restore only files changed by that installation and retain user data unless the
owner explicitly requests deletion. No such operation is performed by these docs.
On any failed activation after possible hook exposure, exit NMS normally before
another attempt. Generic no-retry logs do not prove that no hooks were published
(accepted C3). Never unload/retry/revert individual hooks in the running process.

Startup/preparation/UI/health logs can include local paths, filenames, identities
and opaque runtime addresses. Treat raw diagnostic/review/rollback archives as
private evidence; prepare a separately selected public issue excerpt if needed.
