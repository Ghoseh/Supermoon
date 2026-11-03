# Music folders and settings

The usual music folder is `Binaries/Supermoon/Music`. Add supported WAV/MP3 files
directly inside it; subfolders are not scanned. Restart NMS after library changes.

Your optional `playlist.ini`, Cache and State are stored beside the payload DLL
under `Binaries/Supermoon`. Cache contains prepared audio; State holds saved
preferences and startup records. Keep these folders and your INI during updates.
Preserve music stored at any external path too.

## Optional configuration

Defaults work without an INI. If no personal `playlist.ini` exists, you may copy
the supplied example and edit it with NMS closed. Never overwrite an existing
personal INI with the example.

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

Relative Music paths are resolved from the mod folder, not the terminal's current
directory. Absolute paths are also supported. Use a stable local library and keep
original music independently backed up.

CacheMiB accepts 16–65536 MiB. It covers the Cache folder, including temporary
preparation data and unknown files, rather than original music or every diagnostic
file. Unknown or in-use entries can prevent new cache entries. Incomplete folder
scans preserve cache data; a complete scan can remove obsolete managed pairs.

Normalization accepts finite TargetLUFS values from -60 to 0 and TruePeakDBTP
from -20 to 0. It applies constant gain, not dynamic compression or limiting.
Invalid settings may prevent preparation; check the preparation log.

Shortcuts use unique Ctrl+Shift+F1–F24 combinations, excluding F12, or None.
Plain F6–F9 are not the defaults. Overlays, focus and other bindings can interfere.
Invalid optional keyboard/notification settings disable that feature and are logged.

Shuffle preference persists, but song position/history does not persist between
sessions. The two shuffle cues are embedded notifications, not selectable songs.

## Setup files and removal

The installation guide lists the exact DLL, marker and bank paths. Keep the staged
setup folder together with its recipe and tools, and keep matching recovery records.
Do not rename the installed marker or bank folders. Use the same release's tools
to inspect, recover or disable the installation with NMS closed.

The menu bank also changes built-in ByteBeat audio. Removing only the DLLs leaves
that effect installed. Preserve unrelated files and original game banks.
