> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Phase 1 controller checkpoint — acceptance incomplete

Scope: A1-M1 and A1-M2 only, from Phase 0 commit
`0f42fd9436dfc3ab5c306614def002100be6566f` on the existing
`remediation/v1.0.3-phase0` branch. No release is prepared or qualified here.
All sealed release files, original evidence, installed binaries/bank and user
configuration remain unchanged. Phase 2 has not begun.

## Production changes

Only `source/music_control_prototype.hpp` and
`source/playback_intent_prototype.hpp` change. Adapter callbacks, Core, Engine,
native bridges/clone layouts, recovery wrappers, bank implementation, admission
and identity checks remain unchanged.

`PlaybackIntent::invalidate(captured)` atomically clears only the captured active
word. It retains its monotonic generation, so the same generation cannot revive.
If a newer commit has already replaced that word, invalidation does nothing.
The existing native observation uses the same exact-word compare/exchange;
source matching, retired tickets and the permanent terminal-identity latch retain
their original behavior. Recovery never resets that latch or recycles an identity.

The canceled generation marks the boundary of an observed failed playback epoch.
A strictly newer, validated Core commit is the next playback epoch; there is no
automatic replay on initialization/liveness return. This uses the existing global
monotonic commit identity, with no new native pointer, timer or background thread.
As before, callers must publish only validated playback commits. The controller
responds to failures it observes; it cannot detect an entirely unobserved native
engine reset between control opportunities.

`MusicControl::pump()` now:

1. Returns without an SDK call when initialization is unavailable, invalidating
   current playback intent and the applied-decision cache. Initial unavailability
   does not permanently close future valid commits.
2. Cancels an observed non-live active generation. Liveness returning alone cannot
   silently revive it. Samples use owned source indices and existing predicates.
3. Submits the existing parameter, scope, curve and nominal durations. A failed
   call invalidates only its captured generation, preserving any newer commit.
4. Checks initialization and resamples current intent/liveness after submission.
   Only a successful, still-matching decision is cached as acknowledged.
5. For a failed or superseded enabling call, attempts one immediate neutral
   compensation if the SDK is usable. It revalidates compensation success against
   current intent; a custom commit arriving during compensation remains active
   and requires a subsequent fresh command instead of being cleared or falsely
   acknowledged as suppressed.

There are at most two SDK calls per pump and no new loops, waits or allocations.
The existing try-once pumping guard still rejects nested pumps; the outer pump
rechecks the mutations made by those callers. A failed or mismatched result stays
uncached, representing outstanding work for the next qualified control opportunity.
The original setter signature, values (-35 suppression/-48 neutral), nominal
150/750 ms durations and curve remain. Immediate compensation uses the existing
fallback's 0 ms neutral setting. No SDK work is added to provider/audio callbacks.

## Validation

Every original Phase 0 test/assertion file is unchanged, including all M1/M2,
dependency-policy and cleanup-liveness conditions. Eleven new cases supplement
the original controller cases. The supported runner adds that suite without
changing selection or handling of any existing case; failures remain failures.

Both complete remediation runs (normal and UBSan) return 1:

| Suite | Passing cases | Failing cases | Controlled timeouts |
| --- | ---: | ---: | ---: |
| Original controller | 17 | 0 | 0 |
| Additional controller recovery | 11 | 0 | 0 |
| Actual adapter/release | 3 | 3 | 0 |
| Actual native recovery wrappers | 3 | 0 | 1 |
| Dependency policy | 3 | 2 | 0 |
| Total per run | 37 | 5 | 1 |

There are no harness errors or UBSan diagnostics. All nine previously failing
controller cases pass; the three original final-release cases still fail. The
additional cases exercise first-ready fresh Play without an intermediate pump,
failure/invalidation concurrent with newer native/custom commits, initialization
loss during submission/compensation, failed compensation retry, liveness recovery
without stale revival, and bounded work under continuous generation changes.
Logical intent and last successfully applied parameters are checked separately.

All 23 production recipe units compile individually with the pinned Zig compiler
and production definitions. C++ uses O2/Wall/Wextra/Werror; no DLL is linked.
The normal adapter test still calls the actual native-unlock bridge exactly once
per fixture control update and checks that provider observation never calls SDK.
Unchanged assembly/layout/publication hashes are preserved; this is not a new
native disassembly, asynchronous unwind, deadline or legal-caller qualification.
The C meter and assembly are not UBSan instrumented. No live test is performed.

The complete existing public subset passes normally (qualified release proxy
supplied for wrong-host loading) and under UBSan (optional proxy loading omitted):
eleven suites plus the final parser guard corpus, 25 successful compile/run
receipts each. Each parser run reports 595 checks, zero failures, zero audited
allocations and guard pages enabled. Stream tests report 100 synthetic EOF songs,
20 shuffle cycles, zero callback allocations and 50 worker lifecycles without a
handle leak. The public endurance suite is telemetry simulation, not live endurance.

## Unresolved final-release obligation

The three unchanged failures are:

- `final_provider_stop_no_release_opportunity`
- `last_source_retired_no_release_opportunity`
- `failed_final_release_no_later_callback`

All end with logical suppression false but -35 still successfully applied.
The first ends immediately after the provider observation, which cannot call SDK.
The second retires the source through the actual Term wrapper without a later
control unlock; its native calling phase is not established as an SDK caller.
The third preserves the actual failed neutral result and has no later opportunity
to retry. A subsequent real control unlock does repair release, as separate
passing cases demonstrate; that does not satisfy callback cessation.

SDK calls remain confined to the existing selected control-unlock paths. The
existing writer worker is independent of provider traffic but is not a qualified
SDK calling context: retained current-build native packet tests explicitly exclude
actual queue processing, lifetime and legal host phase. Moving calls there would
add an unproved native concurrency/lock contract. Retaining a native stream pointer,
calling from provider/Term, adding shutdown hooks, or pumping until success would
violate the authorized safety constraints or conceal the missing opportunity.

No arbitrary later pump or altered test deadline was added. A reliable independent
release source still needs to be established and tested before A1-M2/Phase 1 can
close. Failures at a final opportunity also require that service to retry safely.
M3 and H1 retain their separate release/qualification holds and tests.

## Risks and handoff

The SDK fixture models synchronous successful application and no application on
failure. Actual queue ordering, interpolation and audible restoration remain
unqualified. Immediate neutral compensation on a superseded enable needs later
listening validation. A change after the last control-path recheck still needs
another service opportunity; bounded compensation is not a universal release
guarantee under arbitrary concurrent changes or persistent SDK failure.

No new DLL pair, executable/bank/payload pin, installation, configuration, shutdown
contract or publication is supplied. Existing build recipes still qualify the
sealed v1.0.3 hashes and must not be repinned merely to deploy this checkpoint.
Private raw receipts, unchanged failing cases, preservation inventories and the
checkpoint patch are retained in the separate Phase 1 evidence directory.
Stop at this partial checkpoint and wait for owner review before further phases.
