# Supermoon v1.2.1

Play your own WAV and MP3 music through No Man's Sky's personal ByteBeat player.
This maintenance update improves usable music volume and adds shortcut choices
without requiring Shift. Steam NMS **build 26540391**, Windows x64 desktop only.

## Install or upgrade

Close NMS, verify the download checksum, and extract the complete ZIP to a
persistent folder outside the game. Run **Setup.cmd** for automatic installation.

- New installation: choose **Install**.
- Published v1.2.0: choose **Upgrade v1.2.0 to v1.2.1**, then type `UPGRADE`.
- Require verified completion and Status showing `candidate_present`.

Setup uses built-in Windows PowerShell 5.1; no Python or separate compiler is
required. Keep the extracted setup folder and recovery records. Do not extract
directly over the game or replace an existing installation's DLLs manually. See [installation](INSTALL.md)
and the [v1.2.0 upgrade guide](docs/UPGRADE_1_2_0.md).

For a fresh inactive game, **ManualSetup.cmd** can instead prepare verified files
outside the game for you to copy yourself. It also verifies copied files and
checks recognized files before manual removal. See [manual installation](MANUAL_INSTALL.md).
Both routes use the same v1.2.1 DLLs and require built-in Windows PowerShell 5.1.
Manual preparation creates no automatic recovery records and does not change the
game. Keep automatic Setup.cmd for existing installations and v1.2.0 upgrades.

**Updating a manual installation requires removing its four activation files
first, then installing the new release. In-place manual updates are unsupported.**
Keep Music, playlist.ini, Cache, State and logs; never delete the whole Supermoon
folder. To switch to automatic setup, remove/verify those four files first, then
use Setup.cmd Install. See the manual guide's update and removal steps.

Add songs directly to `Binaries/Supermoon/Music`, restart, select Supermoon in
personal ByteBeat and Play. First-time registration can need a normal restart.
Existing music, prepared cache and saved preferences are preserved. Setup creates
a complete `playlist.ini` on fresh installs. Upgrades keep your settings/comments,
back up the original INI and add only missing options with their defaults.

## Volume

Personal ByteBeat voice gain now defaults to **+3 dB** when the setting is absent.
In `Binaries/Supermoon/playlist.ini`:

```ini
[Audio]
VoiceGainDb=3.0
```

Choose a decimal from **0.0 to 6.0 dB**. `0.0` restores neutral gain. Explicit
existing values are preserved; invalid values fall back to neutral. Restart NMS
after editing. Use normal game volume controls for listening level.

This adjusts the personal voice before NMS's existing effects. It preserves
normalization and prepared PCM, adds no compression, and does not adjust shared
audio buses. Native personal ByteBeat can inherit the gain. Guard checks may skip
an increase, and quiet sources or the game's mix can still limit the result.

## Keyboard controls

Defaults remain Ctrl+Shift+F6 Previous/restart, F7 Next, F8 toggle shuffle and
F9 shuffle-and-advance. Each action can use Ctrl with optional Shift and/or Alt:

```ini
[Keyboard]
Enabled=1
Previous=Ctrl+F6
Next=Ctrl+F7
ToggleShuffle=Ctrl+F8
ShuffleAndAdvance=Ctrl+F9
```

For example, `Next=Ctrl+Alt+F7` is also supported. `None` disables an action.
Use different function keys for each action. F12 and Alt+F4 combinations are
refused. Native keys are not blocked: Shift may boost/run, Alt may activate
cockpit free look, and overlays/custom bindings can interfere. Extra modifiers
must match exactly. Restart after editing. No volume hotkeys are added.

See [configuration](docs/CONFIGURATION_AND_LAYOUT.md) and `playlist.example.ini`.
All 12 supported settings are included automatically. Edit the installed
`Binaries/Supermoon/playlist.ini`; never extract the ZIP over an existing game.

## Compatibility and limitations

Playback is mono. Press Play manually after travel/reload if native playback
stops. Another winmm loader or audio-bank override can conflict. Other executable
builds, VR and other stores are unsupported. Previous lifecycle/resource issues
are unchanged; see [troubleshooting](docs/TROUBLESHOOTING.md). The gain precursor
was owner-tested in game; final default handling, shortcuts and upgrade procedure
were tested offline. This is not an endurance qualification.

## Credits and license

MIT, copyright Malik Seals and Supermoon contributors. Required third-party
notices and separate notification-audio terms are included. Development used
OpenAI Codex with owner testing and direction. No music or game audio is shipped.
No Man's Sky/ByteBeat belong to their rights holders; this is an independent mod.
