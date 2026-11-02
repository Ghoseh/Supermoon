# Troubleshooting

**Original track or no custom audio:** verify the exact supported executable and
DLL hashes, mod settings, marker and folder layout. Select Supermoon in the personal
player. Cold cache preparation happens during startup and can delay availability;
start playback after preparation. Inspect startup/preparation logs under Supermoon
including bytebeat-startup-latest.log in Supermoon. A refusal requires normal exit
before retrying; do not hot-reload or swap DLLs in a running game.

Version 1.0.3 also requires the locally generated audio override. With the
matching DLL pair installed and NMS closed, run the setup command in INSTALL.md.
An absent, altered or wrong bank refuses payload activation. Do not substitute
a bank from another game build or mod.

**After travel or reload:** if native ByteBeat stopped, manually press Play.
Automatic resumption is intentionally absent. If native playback continues, music
may continue. Persistent failure after a manual restart should be reported with a
small reviewed log excerpt, not a private full diagnostic archive.

**Missing songs:** top-level supported WAV/MP3 only; filenames supply titles.
Use a stable local Music folder and restart after changes. Interrupted discovery
has accepted finding C1 and may omit songs/evict decoded cache; a clean scan can
rebuild. Keep originals. See SUPPORTED_FORMATS.md for encoding and strict WAV
length requirements. Re-export inconsistent WAVs as separate files; Supermoon
does not repair headers. No claim is made that every WAV is supported.

**Cache fails or storage fills:** leave room for source copies, final PCM and
temporary preparation passes. CacheMiB includes legacy/unknown files and scratch;
leased or unmanaged entries may prevent admission. Default 3072 MiB is a cache-root
budget, not a bound on total music or diagnostic storage. Do not delete an active
cache or edit music while preparation/playback is using it.

**Audio quality:** playback is mono. Normalization is constant gain, not a limiter.
Native/game/OS gain and original recording quality can affect output. Background
soundtrack suppression and Discoveries persistence were owner-tested in 1.0.3.
Brief empty-buffer windows on restart/navigation and an unconfirmed historical
vocal-distortion report remain documented; no decoder fix is claimed.

**Keys or display:** Ctrl+Shift is required for default F6â€“F9 bindings. Focus and
overlays may interfere; UI guards can retain native text. See CONFIGURATION_AND_LAYOUT.md.

Logs can contain personal paths and song names. Review/redact a minimal excerpt
before sharing; never post complete music, caches, PDBs, saves or diagnostic archives.
