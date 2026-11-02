# Supermoon update rules

- Supermoon v1.0.0, source commit d5ff3ca, is the behavior reference. Keep playlist, normalization, controls, recovery and UI behavior unchanged while adapting the native profile.
- Target only NMS Steam build 26295378, executable SHA-256 d393c6782d29e1915854c0d5cf42505c8093740c05dc53705dbed7e0d1d4ac37, 89,489,480 bytes.
- Preserve executable/site/payload identity gates. Unknown builds must refuse activation.
- Keep game executables, native instruction dumps, private fixture data and personal music outside this repository.
- Record actual builds, tests and live results in docs/MIGRATION_STATUS.md. Never claim unperformed checks or transfer the original release's endurance result to this pair.
- Readiness checks and local packaging do not authorize a remote push or public release. Publish only on the owner's explicit instruction.
- The owner completed the live acceptance checks on 2026-11-01. Preserve the installed matching pair and leave the current game session running. Keep private test evidence outside this repository; publication remains a separate owner instruction.
- The owner authorized publishing v1.0.2 as the hook compatibility update, then investigating menu interruption and soundtrack overlap. Keep those follow-up behavior changes separate from this tested pair.
