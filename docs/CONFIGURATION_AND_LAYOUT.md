# Configuration and music folders

Setup creates a complete `Binaries/Supermoon/playlist.ini` automatically. All 12
supported options are present at their defaults. On upgrades, existing values
and comments are preserved; only missing options are added after an exact backup.
Restart NMS after changes. Music, Cache, State and logs are preserved.
Edit the installed file after setup; no need to add settings manually.

```ini
[Playlist]
Music=Music
CacheMiB=3072
[Audio]
VoiceGainDb=3.0
[Keyboard]
Enabled=1
Previous=Ctrl+Shift+F6
Next=Ctrl+Shift+F7
ToggleShuffle=Ctrl+Shift+F8
ShuffleAndAdvance=Ctrl+Shift+F9
[Normalization]
Enabled=1
TargetLUFS=-16
TruePeakDBTP=-1
[Notifications]
ShuffleSounds=1
```

Music paths are relative to the installed Supermoon directory; absolute paths
are supported. Add supported WAV/MP3 files directly, without subfolders. Cache
holds prepared audio; State holds preferences/startup records. Keep originals.
CacheMiB is 16..65536, default 3072; occupied/unknown files can limit preparation.

VoiceGainDb accepts finite decimals from 0.0 to 6.0, such as 1.5 or 4.5. Missing
setting defaults to 3.0; explicit 0.0 gives neutral behavior. Invalid/empty values
fall back to neutral. No live reload or volume hotkeys. Gain affects the verified
personal ByteBeat voice; native personal ByteBeat can inherit it. Guards can
skip an increase. No PCM multiplier, added compression/limiter or shared-bus gain.

Each keyboard action accepts Ctrl with optional Shift and/or Alt followed by
F1..F24, except F12 and Alt+F4 combinations. Order/case/spaces do not matter;
duplicate modifiers, unknown tokens and duplicate function keys refuse. `None`
disables an action; Enabled=0 disables keyboard controls. Invalid optional
settings disable that feature and are reported in the preparation/startup logs.
Examples: Next=Ctrl+F7, Next=Ctrl+Alt+F7, Next=Ctrl+Shift+F7. Native input is not
blocked: Shift can boost/run, Alt can activate cockpit free look. Extra modifiers
must match exactly. Overlays/custom/native bindings can still interfere.

Optional normalization stays unchanged:

```ini
[Normalization]
Enabled=1
TargetLUFS=-16
TruePeakDBTP=-1
[Notifications]
ShuffleSounds=1
```

TargetLUFS accepts -60..0 and TruePeakDBTP -20..0. Constant normalization preserves
dynamics; no new compression is introduced by this release. Invalid processing
settings can stop preparation; inspect preparation-latest.log. Shuffle preference
persists, song position/history does not persist across sessions.
