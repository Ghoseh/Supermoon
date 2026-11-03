> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Phase 5: private candidate preparation

Provisional version: **1.0.4-rc1-private**, from checkpoint `524d064`.
This is an evaluation artifact, not release-ready. No installation, NMS launch or
attach, Phase 6 work, or publication is authorized by this checkpoint.

Production cleanup changes only the empty-library error's stale "experimental"
wording and corrects comments distinguishing the control-unlock pump from provider
observation. The error category/control flow is unchanged; tooling that matches the
old exact message may need updating. A Python import with no uses is removed.
Retained experimental/support helpers are kept: some remain test inputs or
behavior/provenance references, and removal has no demonstrated runtime benefit.
Native hooks, architecture, ownership and synchronization are unchanged.

The proxy's payload size/SHA-256 is deliberately repinned to the new matching DLL.
Recipe/checksum/manifests identify the private pair. Executable, native-site/profile,
bank identity and patch-plan requirements are unchanged. The public build wrapper
continues to verify existing pins and refuses mismatch; it never silently rewrites them.

Docs describe ordinary desktop setup with no launch arguments, unrelated mods
with bank-conflict limits, required generation/enablement, staged clean/update paths,
durable recovery and two-component removal. No personal INI ships. Installation ZIPs
include the referenced documentation. Older sealed versions remain untouched.

A1-M2 still has three original failing final-release assertions. No independent
SDK service path is established. A1-H1/Historical U1 still has the original synthetic
self-wait; NMS reachability remains unproven. These are a release blocker and a
qualification hold, respectively, without automatic acceptance. Original tests and
conditions remain unchanged. Finite capacity/manual recovery and other limits remain.

Phase 5 validation/artifact hashes are recorded with the external private
`PHASE5_REPORT.txt`, `TEST_RESULTS.json`, `CANDIDATE_VERIFICATION.json` and checkpoint.
See QUALIFICATION.md for offline scope. Historical owner listening and endurance do
not qualify this pair. Final listening, effective bank selection, native cleanup
contracts, live headroom and release approval remain outstanding.
