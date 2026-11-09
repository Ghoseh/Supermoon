Supermoon v1.2.0 - Windows x64 desktop Steam NMS26540391

1. Close NMS; keep Steam updates and other file writers idle.
2. Verify the ZIP's trusted SHA-256 and extract the complete archive into persistent
   staging outside game/TEMP. Never merge it directly over the game folder.
3. Double-click Setup.cmd. Confirm/enter the root containing Binaries and GAMEDATA.
4. Choose Install and type INSTALL for the target. Require verified completion,
   then inspect Status (candidate_present, supported executable, verified journal).
5. Add songs to the INSTALLED Binaries/Supermoon/Music folder (create if absent),
   or retain your configured library. The ZIP's empty Music folder is a layout
   reference; setup does not copy songs/configuration from staging.
6. Launch normally, enable mods/Supermoon in normal settings, select Supermoon in
   personal ByteBeat and Play. First registration can require normal exit/restart.

Fresh setup/recovery needs built-in Windows PowerShell Desktop 5.1 and Framework
support, not Python, an external compiler or installer EXE. Keep this whole stage
and recovery backups. Read Supermoon/INSTALL.md and SETUP_SECURITY.txt.
Existing legacy recovery can need matching old Python tools. Different policies
must not merge/relabel records. Never restore old incompatible RC DLLs on 26540391.

Setup preserves personal music, INI, Cache, State, settings/logs and unrelated files.
Runtime replaces latest logs and updates its data: archive before another launch.
No bank/music assets ship. The bank is generated locally from verified game input.
Uninstall/recovery: with NMS closed, matching Status then deliberate Disable,
then Status and verify all four resources absent, including the bank. Keep records.
On failure do not force files/hashes or delete whole folders; see INSTALL.md.
No AV disabling, persistent-policy change, silent elevation or downloads.

Known unresolved M2/H1 and RADAR_PRE_LEAK_64 are documented in release notes and
known limitations. This pair passed one focused live session, not a four-hour
endurance test or every possible shutdown schedule. Manual Play after travel is
intentional. Brief gaps, mono output and finite session storage remain limits.
