# Changelog

## Manual-tools revision 1 (v1.2.1 runtime unchanged)

- Add optional fresh-install preparation and verification through ManualSetup.cmd.
- Retain automatic installation, v1.2.0 upgrade and recovery through Setup.cmd.
- Preserve personal INI and refuse manual replacement of automatic installations.
- No changes to the published runtime DLLs or playback behavior.

## 1.2.1

- Add bounded personal ByteBeat voice gain (0..6 dB, default +3 when absent).
- Add configurable Ctrl-only and Ctrl+Alt function-key shortcuts; retain Ctrl+Shift defaults.
- Add verified public v1.2.0 upgrade, archived old records and independent inactive rollback.
- Retain Python-free setup, existing normalization and Steam NMS build 26540391 gating.
- Supply complete 12-setting playlist.ini; safely complete existing INIs with
  original-file backups and unchanged existing values/comments.

## 1.2.0

- Support Steam NMS build 26540391 and Python-free Windows setup.
- Preserve personal data and version-specific recovery. Earlier releases unchanged.
