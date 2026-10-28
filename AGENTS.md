# Supermoon update rules

- Supermoon v1.0.0, source commit d5ff3ca, is the behavior reference. Keep playlist, normalization, controls, recovery and UI behavior unchanged while adapting the native profile.
- Target only NMS Steam build 26271400, executable SHA-256 d1c7155764271eff0733c37d1849e685a11da3d54a41d685d7898fc0aad83a87, 89,491,528 bytes.
- Preserve executable/site/payload identity gates. Unknown builds must refuse activation.
- Keep game executables, native instruction dumps, private fixture data and personal music outside this repository.
- Record actual builds, tests and live results in docs/MIGRATION_STATUS.md. Never claim unperformed checks or transfer the original release's endurance result to this pair.
- No public release or remote push is part of this repair task.
