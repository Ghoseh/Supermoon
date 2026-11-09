# Supported music files

Use WAV or MP3 files directly inside the configured Music folder. Subfolders,
live library refresh, OGG and FLAC are not supported. Restart the game after changes.
Track titles come from filenames rather than embedded song metadata.

## WAV

Supported inputs are little-endian RIFF PCM at 8/16/24/32 bits, float32/64 and
supported extensible variants, mono or stereo, at 8–384 kHz. Not every WAV format
or container variant is supported.

RIFF and data chunk sizes must be finalized and consistent. If a file is rejected,
check the preparation log and re-export to a supported format as a separate file.
Keep the original. Supermoon does not repair malformed WAV headers automatically.

## MP3

Supported inputs are Layer III MPEG-1/2/2.5, mono or stereo. Layer I/II, free-format
files and streams that change format are rejected. Compressed files are limited
to 512 MiB, one million frames and recognized tags of up to 16 MiB.

Recognized gapless metadata may remove padding; unfamiliar metadata may leave
padding. Malformed or unsupported files may be refused. Keep original recordings
and check the log if preparation fails.

## Playback output

Supported files are prepared as 24 kHz mono audio; stereo is downmixed. Optional
normalization applies constant gain with peak checks, not a compressor or limiter.
Prepared cache files are not replacements for your original music.

The reserved names `shuffle on.wav` and `shuffle off.wav` are excluded from song
discovery. Shuffle notifications are supplied separately within the mod.
