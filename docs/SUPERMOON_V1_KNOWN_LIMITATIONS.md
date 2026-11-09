# Supermoon v1.2.0 known limitations

Read RELEASE_NOTES.md for exact unresolved M2/H1 and the RADAR_PRE_LEAK_64
resource observation. The focused current-pair test does not fix these findings
or provide four-hour endurance/all-schedule qualification.

# Known limitations

## Background soundtrack after Stop

Under some conditions, the game's background soundtrack may remain quieter after
custom playback stops. Normal restoration worked in listening checks, but it is
not guaranteed after every Stop. The frequency and duration are not established.

If this happens, preserve useful logs and exit normally before trying a fresh
session. Repeatedly changing controls should not be treated as proof the problem
was fixed. No universal recovery sequence is established.

## Potential freeze during audio cleanup

A potential cleanup freeze has been reproduced in isolated tests. Whether ordinary
NMS gameplay can trigger that condition is unproven, and no gameplay occurrence
was confirmed in the reviewed tests. Normal exits during testing do not establish
that every game condition is safe.

If the game freezes, preserve available logs after exit and report the preceding
actions. Do not replace DLLs or banks while the game is running.

## Playback and session limits

- Travel, reload or native Stop can stop ByteBeat. Press Play manually afterward;
  automatic resume is not provided.
- Brief restart/Next gaps remain; seamless playback is not guaranteed.
- Audio-object storage is limited per game process and is not recycled during that
  process. Long-session headroom and the game's response to exhaustion are not
  fully established. If a capacity warning appears, exit normally and restart.
- Songs are scanned at startup, directly inside the configured folder. No live
  refresh or subfolder scanning is provided.
- Output is 24 kHz mono. Supported WAV/MP3 formats are described separately.
- Shortcut focus/overlay behavior, title length and diagnostic storage are bounded.

## Compatibility

Only Windows x64 desktop NMS Steam build 26540391 is supported. VR and other
stores/builds are unsupported. Mod-manager deployment, multiplayer compatibility
and arbitrary packed audio-bank priorities have not been established.

Unrelated mods may remain enabled. Another winmm.dll loader or incompatible
init.bnk override conflicts. The setup checks cannot detect every packaged conflict.

The menu workaround also affects built-in ByteBeat tracks and continues while
the bank override is installed. Disabling DLLs alone does not remove that effect.

## Installation and recovery

Keep the game closed and other file writers idle. Setup preserves personal files,
but the four program-file changes are not one atomic operation and power-loss
recovery is not guaranteed. Keep independent verified backups and matching tools.
Different versions' recovery records must not be mixed or edited to bypass refusal.

Normal runtime cache maintenance can remove obsolete managed entries after a
complete scan. Original music is stored separately and must be preserved.

Engineering documentation retains detailed test results and unresolved technical
findings. This player guide does not change those findings or claim they are fixed.
