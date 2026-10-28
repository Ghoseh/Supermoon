# NMS build 26271400 compatibility repair — 2026-10-28

Updated native function, cell, global, return-site, relocation and executable profiles. Preserved music, playlist, normalization, controls and recovery behavior. Built and passed offline regressions. Owner-confirmed custom playback, Next, full-song advancement, teleport/warp recovery and warm relaunch passed.

# Changelog

## 1.0.0 — prepared 2026-10-16

- Integrated WAV/Layer III MP3 playlist, disk-backed preparation/cache, optional
  loudness normalization, filename titles, keyboard navigation and shuffle cues.
- Event-agnostic source-ticket recovery: after native playback stops, manual Play
  can restore custom audio across repeated source lifecycles; no automatic resume.
- Independently reviewed startup XML, MP3 error propagation, shuffle synchronization,
  cache quota recovery, meter cleanup and allocator-last hook-publication corrections.
- Exact RC1 binaries retained after comprehensive audit, 4h00m24s Steam endurance
  and warm cache relaunch. No runtime rebuild for public packaging.
- MIT selected for original source; complete third-party notices and portable build
  instructions supplied. Accepted C1/C2/C3 and endurance observations remain open;
  see known limitations. Publication requires the owner's final approval.
