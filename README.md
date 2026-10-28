# Supermoon compatibility update — NMS build 26271400

Play your own supported WAV and MP3 music through the personal ByteBeat player's
Supermoon track in No Man's Sky. Supermoon prepares a disk-backed playlist,
normalizes loudness, displays filename-based titles and supports shuffle/navigation
shortcuts. Audio is rendered as 24 kHz mono, with stereo inputs downmixed.

## Requirements and compatibility

- Windows x64 and the pinned Steam NMS executable described in docs/BUILD_AND_PROVENANCE.md.
  This release checks its SHA-256; a matching general game version name is insufficient.
- Enable game mods through NMS's own settings. Unknown executables or malformed/disabled
  mod settings refuse activation. No extra mod loader is bundled.
- Use the personal ByteBeat player and select the native Supermoon track.
  Other tracks retain native audio; arbitrary base-speaker routing is not guaranteed.
- Another mod using Binaries/winmm.dll conflicts unless independently integrated;
  proxy chaining is not supplied. Multiplayer/other-mod compatibility is not certified.

## Install and use

Follow [INSTALL.md](INSTALL.md). Put your own music in `Binaries/Supermoon/Music`.
Do not overwrite an existing proxy, configuration or music library without a backup.
Cold startup prepares cache and may take time before custom audio becomes available.
The library is scanned at startup; exit and relaunch after changing songs.

Start the personal ByteBeat Supermoon track normally. Travel or a reload may stop
native playback. Supermoon remains stopped until you manually press Play again.
If native playback continues through a transition, music may continue too.
Recovery follows native source lifecycles, not hardcoded travel events.

Default shortcuts: Ctrl+Shift+F6 previous/restart, F7 next, F8 toggle shuffle, F9
one-shot shuffle-and-advance (Ctrl+Shift is required for all four). Controls depend
on gameplay focus and the active player. The two owner-created shuffle sounds are
embedded notification cues, not extra selectable songs.

## Documentation

- [Supported formats](docs/SUPPORTED_FORMATS.md), including strict WAV RIFF/data lengths.
- [Configuration](docs/CONFIGURATION_AND_LAYOUT.md) and [troubleshooting](docs/TROUBLESHOOTING.md).
- [Known limitations](docs/SUPERMOON_V1_KNOWN_LIMITATIONS.md), [accepted audit findings](docs/ACCEPTED_AUDIT_FINDINGS.md).
- [Build instructions](docs/BUILD_AND_PROVENANCE.md), [developer guide](docs/DEVELOPER_GUIDE.md), [tests](tests/README.md).
- [Dependency inventory](docs/DEPENDENCIES.md), [licensing](LICENSING.md), [changelog](CHANGELOG.md).

The updated pair passed static native-profile review and offline regressions.
Custom playback, Next and automatic song advancement passed a live check.
Teleport/warp recovery and warm relaunch also passed; see docs/MIGRATION_STATUS.md.
The original v1.0.0 endurance result applies to its original pair. Accepted defects
and user observations remain documented.

## Credits and license

Original source and documentation: MIT, copyright 2026 Supermoon contributors.
Third-party libraries retain their own terms; include THIRD_PARTY_NOTICES.md and
licenses/ when redistributing binaries. Notification recordings were created by
the project owner; see ASSET_NOTICE.md for their separate scope.
Development, reverse-engineering support, testing and documentation used OpenAI
Codex AI assistance. This packaging pass used GPT-6; a complete historical model
roster is not established. The owner directed development and performed gameplay
acceptance. No claim of AI-generated notification recordings is made.

No Man's Sky and ByteBeat belong to their respective rights holders. This is an
independent mod, with no affiliation or endorsement implied.
