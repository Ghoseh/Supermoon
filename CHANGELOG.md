# Changelog

## 1.0.3 - 2026-11-02

- Restore hook compatibility with NMS Steam build **26385139**, retaining exact executable, native-site and matching-payload gates.
- Keep ByteBeat playback audible through Discoveries and menu transitions.
- Suppress the native soundtrack throughout custom playback, including silent passages, and restore it after Stop or leaving the custom carrier.
- Accept bounded indexed entries in NMS's current mod-settings MXML without relaxing malformed/disabled-setting refusal.
- Generate the required audio override locally from the player's supported game bank; no game bank or playlist music is distributed.
- Confirm the quiet-passage, Stop/native-return, repeated Discoveries, fast-travel and save-reload checks with the owner. Preserve existing manual Play recovery and brief restart/navigation buffer gaps.
- Pass independent native/UI/audio reviews, recovery/publication and public regressions, and reproduce the exact owner-tested DLL pair. Existing resident shutdown limitations remain.

## 1.0.2 - 2026-11-01

- Adapt native audio, lifecycle, allocator, global and UI profiles to NMS Steam build **26295378** (7.11).
- Refresh executable, native-code, readiness and payload identities while preserving unknown-build refusal.
- Preserve playlist, normalization, controls, filename/time/progress and manual recovery behavior.
- Pass native/public/UI/identity checks and reproduce the exact installed DLL pair.
- Confirm custom music, Next, UI, logged automatic song advancement, fast-travel recovery, normal exit and playback after Steam relaunch. Warm activation took 0.605 seconds on the test machine.
- Retain the existing accepted findings and limits; no new four-hour endurance test is claimed.
- Menu-related temporary playback interruption and native soundtrack overlap remain unresolved. This release updates hook compatibility.

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
