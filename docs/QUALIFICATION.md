# Qualification: Supermoon v1.2.1

Steam NMS build 26540391 only. Final hashes are in BUILD_AND_PROVENANCE.md and
build/recipe.json. Gain precursor was owner-tested at +3 dB, with 13,666 recorded
gain applications and reported noticeable loudness improvement. Its 0 dB baseline
recorded no gain registrations or applications. Native personal inheritance was
explicitly accepted. These observations are not final-output peak measurements.

The final pair differs: missing VoiceGainDb now defaults to +3, and inaccessible,
directory or reparse INI paths reject gain neutrally. Explicit 0..6 values remain
unchanged. No new PCM boost, compressor, limiter, EQ or shared-bus adjustment.
The callback/connection/lifetime implementation and keyboard polling/edge/dispatch
code remain unchanged; modifier parsing extends the default-compatible grammar.

Focused offline qualification covers actual startup INI API/default/failure
behavior, neutral and positive target gains, nonaccumulation/reuse/concurrency,
native callback fixture ownership/retirement/cancellation, keyboard parsing and
input edges, existing playback/notifications and activation policy. No live
keyboard test is claimed or required by the owner. The final pair was not run
in NMS; no new audible output/effects or live routing conclusion is implied.

Installation tests use the actual extracted final Windows package with Python
absent from setup-child PATH. They cover fresh installation/status/disable,
explicit public v1.2.0 upgrade with exact published DLLs and matching generated
records, independent new records/verified old archives, preservation, interrupted
phases, sharing failures, inactive rollback, unsupported builds and unknown/corrupt
state rejection. Native game inputs are owner-supplied offline bytes, not executed.

Release receipts outside the source identify final results/counts and artifacts.
Checks also cover reproducible DLLs/ZIPs, exact manifests/checksums, licenses and
private-path/asset exclusion. This is focused maintenance qualification, not a
new comprehensive or endurance audit. Accepted M2/H1 findings and the historical
RADAR observation remain unresolved/unchanged; no old severity or evidence is
reclassified. No universal callback schedule, power-loss durability, hostile-writer
or broad storage/Windows/overlay/VR/multiplayer matrix guarantee is made.

INI distribution revision: complete active/template INI, additive missing-key
completion with original backups and atomic writes, idempotent retry, retained
user configuration on Disable/rollback. These installer-only changes do not
change either qualified DLL. Separate INI and extracted-package tests apply.
