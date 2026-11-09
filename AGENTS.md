# Supermoon public source maintenance

Version/policy 1.2.0 targets only Windows x64 desktop Steam NMS26540391. Preserve
exact executable, native, payload and bank identity gates. Never bypass mismatches.
The runtime source corresponds to the frozen live-tested pair; do not rebuild or
change native logic during packaging. Keep all copyright/license attributions.
No game files, copied-native byte fixtures, music, private paths, caches, logs,
PDBs, credentials or recovery records belong in this source tree/archive.
Use owned disposable fixtures. Preserve user Music, INI, Cache, State and unrelated
data; closed game is mandatory for mutations. Different recovery policies do not
import/relabel records. Old RC DLLs cannot reactivate on 26540391.
Known M2 failures/H1 timeout and RADAR observation remain unresolved. Do not weaken
tests or claim four-hour/endless/all-schedule qualification. See docs/QUALIFICATION.md.
Publication/tag/push and any real installation/game launch require owner authorization.
