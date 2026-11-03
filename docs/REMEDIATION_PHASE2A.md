> Historical checkpoint summary. Current owner acceptance, Phase 6 qualification and target public v1.1.0 status are in [QUALIFICATION.md](QUALIFICATION.md) and [MIGRATION_STATUS.md](MIGRATION_STATUS.md). Earlier blocker/hold statements below retain their original checkpoint meaning.

# Phase 2A — targeted required-bank admission

This development checkpoint starts at `0042307` on the existing remediation
branch. It addresses A1-M3 for supported desktop use. The sealed v1.0.3 release
and installed files are unchanged. No new DLL pair or release is qualified.

## Supported configuration

Ordinary desktop NMS launches remain supported with no additional Steam arguments.
VR is unsupported and is not reliably detected by this policy. No launch argument,
single-mod configuration or manual MXML entry is required. Desktop admission uses
`Enabled`; `EnabledVR` cannot substitute for it and may be absent or false for a
valid desktop entry. A present VR boolean must be unambiguous valid metadata.

Other mods may remain enabled. The automatic resource check is deliberately narrow:
an enabled, named loose mod that supplies `audio/windows/init.bnk` is refused if
its bytes cannot be verified equal to the required loose bank. Unrelated resources,
desktop-disabled mods and byte-identical banks do not cause blanket refusal.
Opaque packages, legacy mod routes, unresolved names/reparse directories and native
resource precedence are not fully inspected. No ModPriority/index ordering is
assumed and no automatic setting, ordering or enablement change is performed.

Supported mod stacks must avoid incompatible overrides of this specific resource.
For packaged mods, check their documented/file inventory for `audio/windows/init.bnk`
and resolve only an actual resource collision while NMS is closed. Do not disable
all mods or blindly alter priorities. Admission, a pinned disk bank and successful
SDK calls are each distinct from proof of which bank NMS actually loads or its
audible effect. Loaded-resource/audible qualification remains later work.

## First-time registration

NMS must have an opportunity to discover/register its own mod entries. The bootstrap
no longer rejects a missing entry before that can occur. Missing settings, a missing
required entry, a temporary exclusive writer or an empty/changing file are pending
states for the existing startup worker. **Pending never authorizes activation.**
The existing 120-second deadline is retained. Two observations must jointly show
native readiness and a validated enabled entry. Pending state does not accumulate
readiness confirmations. Known disable, malformed/ambiguous settings and a known
resource collision refuse activation; final policy is checked again before loading
the payload and independently before lifecycle/controller publication.

Settings are opened read-only with sharing suitable for NMS updates. Each read handle
closes before parsing/scanning; the file is not pinned, created, repaired or rewritten.
The worker does not block NMS's startup thread or its normal mod-registration flow.
If registration is not durably available in the window, Supermoon remains inactive
and records `audio_registration_pending_exit_normally_and_restart`. NMS continues
normally through the forwarding proxy. Do not fabricate an entry to bypass refusal.

Normal first-install procedure for a future qualified candidate:

1. Set up the matching DLL/bank components with the game closed, following the
   existing install procedure. This checkpoint is not an installable release.
2. Launch NMS normally in desktop mode and let it register mods. Use the game's
   normal mod settings to enable Supermoon's audio entry if needed.
3. If registration becomes available during the startup window, the worker can
   proceed. If it remains pending or changes after refusal, exit normally and
   restart after NMS saves its settings. Do not hand-create/edit the MXML entry.
4. Resolve only a specifically identified incompatible bank override; retain
   unrelated mods and personal data.

Read-only pinned-image inspection confirms startup loads settings, scans/reconciles
mod records and reaches a settings-persist path during initialization. The native
schema and retained legitimate MXML include the fields accepted by the new tests.
Owned fixtures exercise absent settings -> empty registry -> NMS-like registered
entry -> two joint confirmations, and an exclusive writer -> completed registration.
No NMS or copied game instructions were executed for this check. Actual first-run
timing, write failures, native load selection and audibility remain live qualification
limits; the fallback is inactive forwarding/normal exit and restart, never implicit
enablement of a missing entry.

## Policy and preserved tests

`global_mods_enabled` retains the original global grammar/policy contract. Public
startup/F1 tests change only their function binding to that name. All fixture bytes,
oracles, expected outcomes and the 595-check corpus remain intact.
`mods_enabled` now checks the required desktop entry using the same bounded parser.
The original `dependency_policy_tests.cpp` is byte-identical, including its missing
and disabled assertions; all five cases pass without rebinding or altered inputs.

The shared parser retains UTF-8/XML-character/reference validation, 64 KiB size,
32 open-element and 16-attribute limits. Required identity is decoded once and
compared as an ASCII case-insensitive Windows mod name, in the correct typed list
record. Duplicate required names, aliased/contradictory fields, conflicting required
indices and misplaced identity refuse. Index zero is not special; absent indices
and normal metadata/Unicode names are accepted. No required-entry heap allocation
is needed. File/path work happens only in cold admission, never audio callbacks.

The bootstrap additionally checks known enabled loose-resource conflicts. Payload
preflight repeats the same settings/conflict policy before its unchanged native-site
and required-bank checks. All executable, payload, native and bank identity pins
remain unchanged. This source checkpoint deliberately retains the sealed pair's
build hashes; no repinning or DLL installation is performed.

## Validation and remaining holds

Both complete normal/UBSan remediation runs return failure honestly:

| Suite | Passing cases | Failing cases | Controlled timeouts |
| --- | ---: | ---: | ---: |
| Original controller | 17 | 0 | 0 |
| Phase 1 additional controller | 11 | 0 | 0 |
| Actual adapter/release | 3 | 3 | 0 |
| Native recovery wrappers | 3 | 0 | 1 |
| Original M3 fixtures | 5 | 0 | 0 |
| Required-entry policy | 28 | 0 | 0 |
| File/first-registration/conflict preflight | 14 | 0 | 0 |
| Total per run | 81 | 3 | 1 |

There are no harness errors or UBSan diagnostics. Required-policy guard-page tests
report zero parser allocations. All 23 production units compile with production
definitions; no DLL is linked. Original controller/native-wrapper assertions are
unchanged. M2's three final-release failures and H1's conditional self-wait timeout
remain unchanged; no lifecycle/shutdown remedy or Phase 2B work is included.

Both complete public runs also pass: eleven suites and the final parser guard
corpus, 25 successful compile/run receipts per normal/UBSan run. Each parser run
reports 595 checks, zero failures, zero audited allocations and guard pages enabled.
The normal run includes qualified release-proxy wrong-host loading; the UBSan run
omits that optional load phase. These runs use the final code after the added
unrelated-dependency metadata case. No new DLL or live game is exercised.

Automatic conflict checks do not qualify arbitrary packed/legacy mod stacks or
establish the native winning resource. Stable configuration is required through
startup; these file snapshots do not provide an atomic transaction with concurrent
file/setting changes, and settings are not continuously enforced after admission.
No live first-install, VR, audible bank or native asynchronous-unwind qualification
is claimed. Private receipts, first-failure reproductions, native inspection evidence,
preservation inventory and the checkpoint are retained outside the repository.
