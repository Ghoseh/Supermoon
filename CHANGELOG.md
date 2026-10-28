# Changelog

## 1.0.1 - 2026-10-28

- Restore compatibility with NMS Steam build **26271400** by updating native function, dispatch, lifecycle, allocator, global and UI hook profiles.
- Update executable, native-code and DLL identity checks. Unknown game builds continue to refuse activation.
- Preserve existing playlist, audio preparation, normalization, shuffle, keyboard controls and manual Play recovery behavior.
- Verify custom playback, Next, full-song automatic advancement, teleport/warp recovery, normal exit and playback after Steam relaunch. Warm activation took 0.651 seconds on the test machine.
- Pass updated native recovery, hook-publication, UI, startup and public regressions. Reproduce the installed DLL pair with the portable build wrapper.
- Support only the executable listed in [build and provenance](docs/BUILD_AND_PROVENANCE.md). Existing limitations remain; the original release's four-hour endurance result does not qualify this new DLL pair.

## 1.0.0 - prepared 2026-10-16

- Integrated WAV/Layer III MP3 playlist, disk-backed preparation/cache, optional loudness normalization, filename titles, keyboard navigation and shuffle cues.
- Event-agnostic source-ticket recovery: after native playback stops, manual Play can restore custom audio across repeated source lifecycles; no automatic resume.
- Independently reviewed startup XML, MP3 error propagation, shuffle synchronization, cache quota recovery, meter cleanup and allocator-last hook-publication corrections.
- Exact RC1 binaries retained after comprehensive audit, 4h00m24s Steam endurance and warm cache relaunch. No runtime rebuild for public packaging.
- MIT selected for original source; complete third-party notices and portable build instructions supplied. Accepted C1/C2/C3 and endurance observations remain open; see known limitations.
