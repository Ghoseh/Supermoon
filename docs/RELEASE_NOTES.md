# Supermoon v1.2.1

- Improved usable custom-music volume with personal ByteBeat voice gain, now
  defaulting to +3 dB. Configure `[Audio] VoiceGainDb` from 0 to 6 dB; 0 is neutral.
- Optional Ctrl-only and Ctrl+Alt keyboard shortcuts. Existing Ctrl+Shift defaults
  remain unchanged; configure each action under [Keyboard] and restart NMS.
- Supported, verified upgrade from the exact published v1.2.0 installation, with
  personal files preserved and independent recovery records.
- Complete playlist.ini supplied automatically: all 12 settings are ready to
  adjust. Upgrades back up the original and add missing options without changing
  existing values or comments.
- Python-free Windows setup retained. Steam NMS build **26540391** only.

Existing explicit gain settings are preserved. Native personal ByteBeat may
inherit gain; normalization, prepared PCM and the game's effects are unchanged.
Native key/overlay conflicts remain possible. Higher gain does not fix every
quiet source or mix. Gain was owner-tested in its precursor; final default,
shortcuts and installer qualification are offline. Previous limitations remain.
## Optional manual installation tools

Automatic Setup.cmd installation, v1.2.0 upgrades and recovery remain supported.
ManualSetup.cmd can prepare verified files for a fresh manual installation.
Manual updates require removing the four activation files before installing the
new release; in-place manual updates are unsupported. Keep Music, playlist.ini,
Cache, State and logs. See the manual installation guide for checked removal
and switching to automatic setup. Runtime DLLs and playback are unchanged.
