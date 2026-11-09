# Troubleshooting

## Setup is blocked or cannot start

Fresh setup uses Windows PowerShell Desktop 5.1, not Python. Extract the whole ZIP
and run Setup.cmd. Read root SETUP_SECURITY.txt for downloaded-script and managed
policy restrictions. Do not disable antivirus or change persistent policy. Keep
the failure message. Legacy recovery needs matching tools, possibly Python;
see NMS26540391_MIGRATION.md. Never force a mismatched game or record.

## The original track plays, or custom music is unavailable

Confirm the supported game build, matching DLL pair, enable marker, bank and normal
NMS mod enablement. Select Supermoon in the personal ByteBeat player. Preparing a
new music library can delay availability. Inspect `bytebeat-startup-latest.log`
and `preparation-latest.log` under `Binaries/Supermoon`.

After startup refuses activation, exit normally before trying again. Do not swap
DLLs or attempt to reload the mod while the game is running.

## The audio entry is missing or disabled

Generate the bank using the staged setup tool, then enable the registered entry
in NMS's desktop mod settings. First-time registration may require a normal exit
and restart after settings are saved. Do not create settings records manually or
add Steam launch arguments to force activation.

## Another mod conflicts

Do not overwrite a foreign `winmm.dll` or an incompatible `audio/windows/init.bnk`.
Keep unrelated mods. The setup tool cannot identify every packed-bank conflict;
blind priority changes or unverified mod-manager deployment are not recommended.

## Setup failed or was interrupted

Keep NMS closed and use the same release's setup tool to check `status`. Preserve
the recovery folder. The bank may already have changed, even if setup returned
`REFUSED_OR_INCOMPLETE`. Choose resume, rollback or disable based on the reported
state and installation guide. They are alternatives, not a sequence to run blindly.

If a record belongs to a different version, keep it and use its matching tools.
Never edit its version or hashes, discard the first backup, or delete the whole
mod folder to force an installation.

## Native music stays quieter after Stop

This is a known unresolved limitation under some game conditions. Historical old-pair return
worked in listening checks; one focused current-pair session passed and every Stop
is not guaranteed. Preserve useful logs
and exit normally before another session. No timing or universal repair guarantee
is established. Do not replace files or repeatedly change controls to hide a failed check.

## The game freezes

A potential freeze during audio cleanup was reproduced in isolated tests; ordinary
gameplay reachability is unproven. Do not assume every freeze has that cause.
Preserve available logs after exit and report what happened before it. Do not
attempt live DLL/bank replacement or remove user files as a repair.

## Music stopped after travel or reload

Press Play manually in the personal ByteBeat player. Automatic resume is not
provided. If manual Play does not recover, retain the logs and report the failure.

## Songs are missing or preparation cannot finish

Use supported top-level WAV/MP3 files and restart after library changes. Keep the
original library on stable local storage. Check the configured folder and free
cache space. Do not delete active cache files. Keep originals and re-export an
unsupported or malformed WAV to a separate file if needed.

## Capacity warning

Audio storage is limited per game process. Exit normally and restart for a fresh
session. There is no established hours-to-exhaustion guarantee. Missing logs alone
do not prove that everything is healthy.

## Short gaps, mono sound or shortcut problems

Brief restart/Next refill gaps remain. Stereo is downmixed to mono. Normalization
adjusts loudness and does not act as a dynamics limiter. Default F6-F9 shortcuts
all require Ctrl+Shift; focus, overlays and other bindings can interfere.

Before sharing a diagnostic excerpt, redact personal paths and song names.
Do not post music, caches, saves, credentials or full diagnostic archives.

## Windows resource observation

RADAR_PRE_LEAK_64 was recorded for NMS during the focused session. Preserve
diagnostics and system observations if it recurs; no cause is attributed to this
mod without comparison/evidence. Do not disable security software or delete user
data as a repair. See RELEASE_NOTES.md.
