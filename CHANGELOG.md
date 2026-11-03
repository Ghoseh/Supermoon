# Changelog

## v1.1.0

### Playback and reliability

- Improve recovery after temporary soundtrack-control failures.
- Preserve usable cache entries when a music-folder scan is interrupted.
- Improve cleanup after an audio-initialization error.
- Add clearer startup, capacity and recovery messages.

### Installation and updates

- Add a staged installer with durable backups and status, resume, rollback and disable tools.
- Preserve personal INI, Music, Cache, State and unrelated files during installation.
- Check that the required audio entry is enabled and refuse known conflicting bank overrides.
- Keep the existing menu audio and soundtrack-suppression features from v1.0.3.

### Limitations that remain

- Native soundtrack volume may not return after every Stop.
- A potential cleanup freeze is reproduced in isolated tests; ordinary gameplay
  reachability is unproven and no gameplay occurrence was confirmed in reviewed tests.
- Manual Play after travel/reload, brief refill gaps and finite per-session storage remain.
- Support remains limited to Windows x64 desktop Steam build 26385139.

Python 3.10+ is required for setup/recovery, not gameplay. No Steam launch arguments.

## v1.0.3 — November 2, 2026

- Restore compatibility with NMS Steam build 26385139.
- Introduce the locally generated audio-bank workaround for ByteBeat menu persistence.
- Introduce native soundtrack suppression through quiet and silent custom passages.
- Support the game's indexed mod-settings format.
- Add local generation of the required bank, making Python a setup dependency.

This was a public release. Its audio features were carried forward into v1.1.0.
Native soundtrack restoration after every Stop is not guaranteed.

## v1.0.2 — November 1, 2026 (America/New_York)

- Update compatibility for NMS Steam build 26295378.
- Preserve playlist, normalization, shuffle, filename display and playback controls.
- Retain manual Play recovery after travel/reload.
- Menu interruptions and native music overlap during quiet passages remained at this release.

## v1.0.1 — October 28, 2026

- Restore compatibility with NMS Steam build 26271400.
- Preserve existing playback features and manual recovery behavior.

## v1.0.0 — October 16, 2026

- Initial release with supported WAV/MP3 playlists, caching, loudness normalization,
  filename titles, shuffle and keyboard navigation through the personal ByteBeat player.

Detailed development and test history is retained separately in the engineering documentation.
