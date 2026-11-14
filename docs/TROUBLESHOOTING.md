# Troubleshooting

- No activation: verify NMS build 26540391, all four managed resources and matching
  Status. Another winmm loader or conflicting bank can prevent operation. Enable
  mods/Supermoon normally; first registration may need a normal restart.
- New library: wait for initial audio preparation before comparing volume.
- Still quiet: check Music volume and VoiceGainDb, restart after INI changes, and
  check source quality. Gain is bounded and may be skipped by safety guards.
- Native personal ByteBeat louder: this can inherit the personal voice gain.
  Set VoiceGainDb=0.0 and restart for neutral behavior. Stop alone is not a reset.
- Shortcuts: default Ctrl+Shift+F6..F9; Ctrl-only/Alt variants are optional. Extra
  modifiers must match. Shift/Alt can still activate game actions; overlays and
  focus can interfere. Invalid bindings disable keyboard controls and are logged.
- Travel/reload: press native Play manually when playback stops. Automatic resume
  is not added. Brief refill gaps and mono playback remain limitations.
- Setup refusal: keep NMS closed, retain records and inspect matching Status.
  Use explicit Upgrade for public v1.2.0. Do not force DLLs or edit recovery hashes.
- Remove: matching Disable, then verify both DLLs, marker and generated bank absent.
  Keep music and recovery records. See [installation](../INSTALL.md).

This maintenance release does not resolve previously documented shutdown/source
recovery edge cases or the historical Windows resource-warning observation.
No all-schedule or endurance guarantee is made. Preserve logs for support rather
than deleting state or unrelated files. Latest runtime logs are replaced during
later sessions; copy them before another launch when investigating a problem.
