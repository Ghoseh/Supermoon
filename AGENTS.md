# Supermoon update rules

- Supermoon v1.0.0, source commit d5ff3ca, is the behavior reference. Keep playlist, normalization, controls, recovery and UI behavior unchanged while adapting the native profile.
- Target only NMS Steam build 26295378, executable SHA-256 d393c6782d29e1915854c0d5cf42505c8093740c05dc53705dbed7e0d1d4ac37, 89,489,480 bytes.
- Preserve executable/site/payload identity gates. Unknown builds must refuse activation.
- Keep game executables, native instruction dumps, private fixture data and personal music outside this repository.
- Record actual builds, tests and live results in docs/MIGRATION_STATUS.md. Never claim unperformed checks or transfer the original release's endurance result to this pair.
- Readiness checks and local packaging do not authorize a remote push or public release. Publish only on the owner's explicit instruction.
- The owner is playing with friends. Complete offline preparation only; do not install, attach to, close or launch the game. Live acceptance is pending until the owner is ready.
