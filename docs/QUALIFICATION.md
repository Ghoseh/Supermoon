# Qualification of the v1.2.0 runtime pair

Scope: Windows x64 desktop Steam NMS26540391, exact pair in BUILD_AND_PROVENANCE.md.
Focused independent static/native fixture review passed for the mapped compatibility
contracts. Offline PowerShell installation/status/disable/recovery was tested with
Python unavailable to setup. Public archive tests cover the actual extracted ZIP;
the accompanying release receipt identifies final hashes and coverage.

One owner-run focused desktop session lasted about11 minutes of sampled game
presence, not four hours. Owner reported all requested functions worked, including
PCM playback/ambience, Stop/Play, Next/Previous/shuffle, teleport/hyperdrive,
reload/source recreation and restored playback. Logs independently recorded32547
provider callbacks,12671587 custom substituted frames and three source retirements,
with zero recorded callback exceptions/fault rows. Three source/parameter/event
slots were used; two rejected late admissions and3485 empty PCM frames were recorded.
They are disclosed counts, not independent proof of an audible fault.
Nine soundtrack-control commands returned acknowledged success. The winning bank
and each travel type were not directly instrumented; audible/travel results are
owner observations. Process exit was observed, not a proof of native draining.

The conditional H1 timeout and three M2 failures remain unresolved, with original
severity and historical findings preserved. The retained native-remediation suite
remains overall FAIL (81 PASS,3 FAIL,1 TIMEOUT). It was not rerun or weakened to package
this release. A new RADAR_PRE_LEAK_64 resource observation was recorded for NMS;
there was no owner-reported crash/instability and no cause attribution. No clean-game
comparison, leak diagnosis or broad resource qualification follows from this run.

Setup is not universally crash/power-loss durable and its cooperative lock cannot
exclude hostile writers. Signed/managed-policy, storage and Windows-version matrices
are limited. Legacy migration requires matching tools and archival, not record
relabeling. These are finite observations and explicit caveats, not universal safety.
Previous Phase 6/four-hour results belong to a different pair and do not transfer.
