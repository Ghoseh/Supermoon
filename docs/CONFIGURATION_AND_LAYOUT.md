# Portable configuration and installation layout

This describes the current portable runtime layout. Current qualification is in QUALIFICATION.md. `<game-executable-directory>` means the directory
containing NMS.exe, independent of the Steam library/drive or developer workspace.

Public package/recovery version is **v1.1.0**, retaining the DLL pair qualified as
**1.0.4-rc1-private** in Phase 6. M2 and H1/U1 are explicitly owner-accepted unresolved
limitations; see [qualification](QUALIFICATION.md). The installed RC record and
matching tools remain RC-specific and unchanged. See the approved
[version policy](V1_1_0_VERSION_TRANSITION.md).

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

Supermoon v1.1.0 also requires the locally derived override at
`<game>/GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk`.
Tools are kept separately in the persistent extracted staging package:

```text
<staging>/
  winmm.dll
  bytebeat-playlist-experimental.enabled
  Supermoon/
    supermoon_playlist.dll
    tools/install_supermoon.py
    tools/installation_safety.py
    tools/prepare_audio.py
    tools/audio_patch.json
    build/recipe.json
    playlist.example.ini
```

The staging tools, patch plan and recipe must retain these relative paths.
See INSTALL.md for generation, backup, conflict handling and removal.

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
Incomplete discovery now blocks all cache removal; a later complete scan can
recover omitted pairs. Stable local music is recommended; enumeration is not an
atomic snapshot or a promise of network/removable-media reliability.
Installation preserves Cache; a complete runtime scan may still evict obsolete
managed pairs. This is separate from original music, which is not removed.
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
(historical C3; stage reporting is now explicit). Never unload/retry/revert individual hooks in the running process.

Startup/preparation/UI/health logs can include local paths, filenames, identities
and opaque runtime addresses. Treat raw diagnostic/review/rollback archives as
private evidence; prepare a separately selected public issue excerpt if needed.

Candidate desktop admission additionally requires a unique enabled registered
Supermoon audio entry, known loose-resource conflict checks and the pinned bank.
No launch arguments are required. VR and opaque-package winning-bank selection
remain unqualified. Tools stay in persistent staging; only four managed resources
are installed. See INSTALL.md for durable recovery and preservation of personal data.
