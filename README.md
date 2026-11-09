# Supermoon v1.2.0

Play your own WAV and MP3 music through No Man's Sky's personal ByteBeat player,
with filename titles, normalization, shuffle and keyboard navigation. Playback is
mono. No playlist music or game audio is distributed.

**Windows x64 desktop Steam NMS build 26540391 only.** Other builds/stores and VR
are unsupported. Fresh installation uses built-in **Windows PowerShell Desktop
5.1** through **Setup.cmd**. No Python, compiler, standalone installer EXE or extra
third-party runtime is needed. Windows/.NET Framework support and writable local
storage are required; managed script restrictions can prevent setup.

## Install and play

1. Close NMS, verify the ZIP hash and extract the complete Windows ZIP into a
   persistent staging folder outside the game and TEMP.
2. Double-click Setup.cmd. Confirm or enter the root containing Binaries and
   GAMEDATA. Choose Install and type INSTALL for the displayed target.
3. Require verified completion and Status showing candidate_present/verified.
4. Add songs directly to the installed Binaries/Supermoon/Music folder (create
   it if absent), or retain your configured library. Restart after library changes.
5. Launch normally. Enable mods and Supermoon in normal game mod settings. First
   registration may need a normal exit/restart. Select Supermoon in personal
   ByteBeat and press Play. No Steam arguments or hand-edited registration needed.

Read [Installation, updates and removal](INSTALL.md), including script restrictions
and legacy transitions. The ZIP's empty Music folder is a layout reference;
setup manages four program resources, not a library copy. Keep personal files.
Defaults work without playlist.ini (relative Music, 3072 MiB cache); copy the
example only if no personal INI exists. Never extract over an existing game folder.

## Controls

Use native Play/Stop. Ctrl+Shift+F6 is Previous/restart, F7 Next, F8 toggle shuffle,
and F9 shuffle-and-advance. Each shortcut includes Ctrl+Shift. Focus/overlays can
interfere. After native playback stops during travel/reload, press Play manually;
automatic resume is not provided. Preparation of a new library can take time.

## Compatibility and limitations

Another winmm.dll loader or incompatible bank override conflicts. Unrelated mods
can remain, but opaque packed-bank priorities, multiplayer, mod managers and a
broad Windows/storage matrix are not qualified. The locally generated menu bank
also affects built-in ByteBeat while installed; remove it along with the DLLs.

Three M2 tests still fail: native music may remain quiet after Stop/retirement.
The conditional H1 cleanup fixture still times out; gameplay reachability remains
unproved. One focused live session passed owner-observed functions, but does not
fix those findings or qualify every schedule. No four-hour endurance claim applies
to this DLL pair. A Windows RADAR_PRE_LEAK_64 resource observation remains unresolved;
no cause has been attributed to Supermoon. See [Known limitations](docs/SUPERMOON_V1_KNOWN_LIMITATIONS.md)
and [Qualification](docs/QUALIFICATION.md). Brief Next/refill gaps, finite session
storage and bounded diagnostics remain limitations.

[Music formats](docs/SUPPORTED_FORMATS.md) · [Configuration](docs/CONFIGURATION_AND_LAYOUT.md) ·
[Troubleshooting](docs/TROUBLESHOOTING.md) · [Release notes](docs/RELEASE_NOTES.md) ·
[Build instructions](docs/BUILD_AND_PROVENANCE.md)

## Credits and permissions

Original code/documentation: MIT, copyright Malik Seals and Supermoon contributors. Existing
copyright attributions and third-party notices remain intact. The project owner's
two embedded notification recordings retain separate terms in ASSET_NOTICE.md;
the MIT source grant is not a standalone recording-reuse license. Development and
documentation used OpenAI Codex assistance under human direction and owner testing.
No Man's Sky/ByteBeat belong to their rights holders; this is an independent mod.
