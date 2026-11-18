Supermoon v1.2.1 - Windows x64 desktop Steam NMS build 26540391

Close NMS. Verify the ZIP checksum and extract completely to a persistent folder
outside the game/TEMP. Run Setup.cmd and confirm the game root containing Binaries
and GAMEDATA. Choose Install for a fresh game, or 8 Upgrade for public v1.2.0.
Type INSTALL or UPGRADE, require verified completion, then inspect Status.
Do not extract over the game or manually replace an existing installation's DLLs. Keep this complete stage
and both versioned recovery stores. See Supermoon/INSTALL.md and SETUP_SECURITY.txt.

Optional fresh-install route: run ManualSetup.cmd to prepare files outside the
game, then follow MANUAL_INSTALL.md to copy and verify them yourself. Automatic
Setup.cmd and its v1.2.0 Upgrade remain supported. Manual preparation also needs
PowerShell; it does not bypass Windows restrictions. Never discard old recovery
records or overwrite personal INI. It creates no automatic recovery records.

Manual updates require removing the four recognized activation files first,
verifying they are absent, then installing the newer release. Do not update by
overwriting active manual files. Keep Music, playlist.ini, Cache, State and logs;
do not delete the whole Supermoon folder. To switch from manual to automatic,
remove/verify those four files first, then choose Setup.cmd Install. Follow the
newer release's instructions for future upgrades. See MANUAL_INSTALL.md.

No Python/compiler/download is required. Built-in Windows PowerShell Desktop 5.1
and Windows/.NET Framework support are needed; managed script policy may refuse.
No elevation, AV disabling or persistent policy change is performed.

Add WAV/MP3 music directly to installed Binaries/Supermoon/Music. Existing personal
files are preserved. Setup supplies a complete playlist.ini automatically, or
backs up your original and adds only missing defaults. Existing values/comments
are kept. Edit the installed file, not the staging copy. VoiceGainDb defaults to
+3 dB; configure 0..6 dB and restart NMS.
Ctrl+Shift+F6..F9 remain defaults; Ctrl-only/Ctrl+Alt alternatives are configurable.
Enable mods/Supermoon normally, restart if registration needs it, select personal
ByteBeat Supermoon and Play. Initial library preparation takes time.

For removal, keep NMS closed, run matching Status/Disable/Status and verify all
four managed resources absent, including the bank. Rollback after fresh setup or
v1.2.0 Upgrade returns to absence, not old DLLs. Keep records/music/INI/Cache/State.
On failure preserve records and use matching Resume/Disable; never edit hashes.
