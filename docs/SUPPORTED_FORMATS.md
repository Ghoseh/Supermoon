# Supported audio and container requirements

Discovery uses top-level `.wav` and `.mp3` files in the configured Music folder.
The default is `Supermoon/Music`, relative to the mod installation. The reserved
`shuffle on.wav` and `shuffle off.wav` names are notification cues, not selectable
tracks. Subfolder discovery, live refresh, embedded metadata titles, stereo output
and other codecs such as OGG/FLAC are not implemented.

## WAV

The implemented reader supports little-endian RIFF PCM8/16/24/32 or float32/64,
supported extensible variants, mono/stereo input and 8–384 kHz sample rates, subject
to its channel/format/alignment/finite-value and bounded-length checks. This is not
a claim that every WAV encoding or container variant is supported.

WAV files must contain correctly finalized RIFF and data chunk length fields.
Some files contain usable PCM samples but are rejected because their headers have
unfinished or inconsistent lengths. Re-exporting the WAV through an audio editor
such as FL Studio may produce a compatible file; the resulting encoding must also
be supported. Supermoon does not automatically repair malformed WAV headers.
Preserve originals and make any user-directed re-export as a separate file.

## MP3

Production uses the patched low-level dr_mp3 Layer III decoder for MPEG1/2/2.5,
mono/stereo, within its supported rates and header/frame/tag/count limits. Layer
I/II, free-format and streams that change format are rejected. Compressed files
are limited to 512 MiB, frame count to one million and recognized tags to 16 MiB.
Recognized Xing/LAME-family metadata may trim padding; unfamiliar metadata can
retain it. MPEG CRC authenticity and rejection of every semantically corrupt
finite stream are not guaranteed.

Mandatory internal decode errors propagate to the wrapper: a failed frame returns
zero samples and is not exposed as a successful complete frame. Valid bit-reservoir
and synthesis state are retained across successful frames. Guard-page, malformed
input, UBSan and known-good reference comparisons cover recorded cases, not every
possible compressed input or an unproven exploitability claim.

## Prepared playback

Supported inputs become disk-backed 24 kHz mono float PCM; stereo is downmixed.
Optional normalization applies constant gain and conservative peak checks, not a
compressor or limiter. Cache identities include source content, path, recipe and
settings. A changed source requires revalidation/rebuilding; cached PCM is not a
replacement for preserving the user's original audio.
