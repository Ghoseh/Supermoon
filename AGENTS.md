# Supermoon maintenance rules

- Preserve the v1.0.0 behavior reference except for documented fixes in CHANGELOG.md.
- Public package/recovery policy is v1.1.0, retaining the exact Phase 6-tested 1.0.4-rc1-private DLLs byte-for-byte. The installed RC remains on its original record and matching tools. Both target only NMS Steam build 26385139: 89,535,560 bytes, executable SHA-256 a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109.
- Preserve executable, native-site, payload and generated-bank identity gates. Unknown builds must refuse activation; never bypass a mismatch.
- Keep game executables, native instruction dumps, private fixtures, music, logs, saves, settings and credentials outside this repository and release archives.
- Record actual checks and their limits in docs/MIGRATION_STATUS.md. Historical endurance results do not qualify a later DLL pair.
- Preserve personal Music, playlist.ini, Cache, State and unrelated mods during updates. Require a closed game before changing installed DLLs or the generated bank.
- The audio bank is derived locally with tools/prepare_audio.py. Distribute the tool and numeric patch plan, never native game audio or the generated bank.
- Live listening requires owner confirmation. Hot unload, exhaustive native draining and universal clean shutdown remain unsupported; do not expand this audio fix into shutdown research.
- Remote publication requires explicit owner authorization. The owner authorized the combined 1.0.3 hook/audio release and completed listening, menu, travel, reload and Stop acceptance on 2026-11-02.

- Phase 6 owner-run desktop qualification completed with both sessions exiting normally. The owner approved Ready with Caveats and explicitly accepted unresolved M2 (original Medium) and H1/U1 (A1 High conditional) risks for the limited desktop scope, subject to final publication verification. Preserve failed tests, evidence and historical audit blocker/hold classifications; do not mark either fixed, harmless or a confirmed gameplay failure.
- Retain the exact Phase 6 DLL pair. The owner approved local v1.1.0 artifact preparation and its separate public policy at ab2e65a. The installed RC record and matching tools must remain intact; never rewrite/import records automatically. See docs/V1_1_0_VERSION_TRANSITION.md. No installed change, game launch/attach, remote upload or release creation is authorized; stop after final artifact verification for publication approval.
