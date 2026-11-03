# Current accepted unresolved findings — v1.1.0

The owner approved Ready with Caveats and explicitly accepted M2 and H1/U1 risks
for the limited desktop release, subject to final publication verification.
The v1.1.0 public policy retains the DLL pair qualified as `1.0.4-rc1-private`.
The installed RC record/tools remain unchanged. Public upload awaits explicit
owner approval. See [qualification](QUALIFICATION.md) and
[known limitations](SUPERMOON_V1_KNOWN_LIMITATIONS.md).

- **A1-M2, original Medium:** three final-release regressions still fail; native
  soundtrack volume may fail to restore after Stop under certain callback conditions.
  Normal restoration passed live checks; universal restoration is not guaranteed.
  No confirmed gameplay failure, likelihood or duration is established. The audit's
  historical release-blocker classification remains preserved; it originated in
  the independent audit, not the owner.
- **A1-H1, original High conditional / historical U1:** isolated cleanup self-wait
  still times out; actual NMS reachability is unproven. No gameplay occurrence is
  confirmed. Both normal exits do not prove exclusion. Historical qualification-hold
  and earlier conditional Medium U1 classifications remain.

Acceptance changes release disposition, not severity, assertions or evidence.
Neither is fixed or proven harmless. C1 discovery and C2 Init cleanup are corrected
in the retained pair under deterministic owned tests; C3 reporting uses explicit
exposure stages. U3 visibility improved without removing finite capacity/headroom
limits. See [Phase 4](REMEDIATION_PHASE4.md).

# Historical accepted noncritical findings for v1.0

Everything below describes the older baseline, including its then-current code
and capacities/diagnostics. It does not override current dispositions above or
transfer historical endurance to the later pair.

The final comprehensive independent audit found no confirmed Critical or High
release blocker. The owner accepted the following noncritical findings for v1.0
and retained the exact audited runtime. These findings remain present after the
completed endurance test; acceptance does not mean they were fixed.

| ID | Severity and evidence | Current behavior and realistic impact |
|---|---|---|
| C1 | Medium, confirmed native Win32 boundary failure; normal and UBSan fixtures | An interrupted directory scan can silently appear complete, omit usable songs and evict their decoded cache entries. A later clean scan recovers and rebuilds. Original music was preserved; no source-song deletion or memory corruption was demonstrated. |
| C2 | Low, synthetic invalid-input cleanup gap | A fault while reading a non-pool parameter before Init's `__try` can leave TLS and endpoint initialization/executing state uncleared if the host recovers the exception. No valid-caller occurrence or ordinary gameplay crash was established. |
| C3 | Low, diagnostic ambiguity | Only activation result 10 explicitly reports partial publication/exit required. Result 16 and later preparation failures can also leave resident hooks while logging a generic no-retry refusal. One-shot activation/residency remain in force; no new incompatible Free route was found. |

C1 is in `playlist/wav_pipeline.cpp::discover` and the obsolete-entry eviction
gate in `playlist/persistent_cache.cpp`. The pinned filesystem implementation can
discard enumeration errors, so empty diagnostics are not a proven discovery-
complete signal. For controlled endurance use a preverified, stable local library
and compare the prepared track inventory. Broad reliance on interrupted, removable
or network-backed discovery is not qualified. This documentation does not add a
runtime completeness check or suppress eviction.

C2 is in `recovery_native.cpp::RecoverySourceInit`: the borrowed-parameter snapshot
precedes the finally-protected scope. Its contract requires valid caller-live
parameters. Later guarded context faults were shown to clean up. No claim is made
that all Init faults or all external invalid pointers are safely recoverable.

C3 spans the activator and bootstrap status reporting. Any refusal after possible
hook exposure requires normal process exit. Never hot-retry activation, unload the
DLL or revert individual dependent hooks while owned objects may exist. The
allocator-last F6 correction remains intact; wording here does not change logs.

## Unverified contracts and qualification limits

These are separate from the confirmed findings above:

- U1, conditional Medium: native lock order, reentrant cleanup and same-owner
  Init/cleanup serialization are not proved for every NMS schedule. The owner
  control path waits for admitted owned calls without a timeout. No NMS deadlock
  was reproduced; destroying admitted storage after a timeout would be unsafe.
- U2, conditional Medium: owned source tickets attribute work after source
  selection. They do not prove all upstream delayed/address-based native work
  selects the correct later incarnation. No concrete wrong-source NMS schedule
  was demonstrated. Same address or numeric ID alone is not lifecycle identity.
- U3, Low–Medium qualification limit: 4096 Source, 8192 Parameter and 4096 Event
  slots are process-wide for intercepted factory sites, retained without recycling.
  Saturation refuses safely, but long/high-count headroom, worst-case scan deadlines
  and every host shutdown schedule remain unmeasured. Health output does not expose
  every registry counter; do not invent missing measurements.
- Filesystem prechecks, opens and removal are not an atomic namespace boundary
  against noncooperating tools. No broader original-data-loss exploit was shown.

The original report, FINDINGS.json, reproduction receipts and historical failures
remain preserved privately. This summary changes no severity or original evidence.
