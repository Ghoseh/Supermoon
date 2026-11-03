# Installation, updates and recovery

This describes the **v1.1.0** public installation/recovery policy, retaining the
exact DLL pair installed and owner-tested as **1.0.4-rc1-private** in Phase 6.
The public recipe version is `1.1.0`. Existing RC installations keep their original
record and matching tools; these instructions do not authorize changing the
owner's installed RC or rewriting its records.
The owner accepted M2 and H1/U1 as unresolved known limitations for the limited
desktop scope. See [qualification](docs/QUALIFICATION.md) and
[the approved version policy](docs/V1_1_0_VERSION_TRANSITION.md).

The sealed v1.0.3 ZIP remains unchanged and has different tools/pins and a live INI.
Never extract it over personal configuration or combine its DLLs with this pair.

## Supported boundary

Use Windows and Python 3.10+ with a supported, persistent local NMS installation.
No extra Python packages, elevation or Steam launch arguments are needed.
Close NMS completely and keep it closed throughout installation and recovery.
Keep Steam updates, other installers, extractors, backup restoration and other
filesystem writers idle. One operator/tool may change the installation at a time.

The tools serialize cooperating operations using an exclusive Windows handle to
the game's `.supermoon-install.lock`. The empty file remains after completion;
process termination releases its handle automatically. A lock or closed-game
snapshot does **not** prevent Steam from starting later, or another program from
changing files/directories. Rechecks narrow those windows; arbitrary concurrent
changes and hostile path replacement are unsupported. Reparse-point paths,
hard-linked managed files and indeterminate process enumeration are refused.

Only four program resources are managed: `Binaries/winmm.dll`,
`Binaries/Supermoon/supermoon_playlist.dll`, the exact activation marker, and
`GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk`.
Personal `playlist.ini`, Music, Cache, State, settings, saves, other mods and
unknown files are never installation destinations. Unknown managed-file bytes
are refused rather than automatically overwritten. This policy recognizes the
exact pair in the accompanying trusted recipe; it is not an arbitrary-version updater.


## Update and migration after approval

An update is staged outside the game; never merge/extract the ZIP over Binaries.
The installer changes only the four named resources, not documentation, tools or
personal data. Keep trusted tools in staging for future recovery. It recognizes
only its exact recipe's DLL pair, marker and accepted banks: it is not a cross-version updater.

For the same v1.1.0 policy, run `status`, then `install --package $stageDir` (or
`resume` for its unfinished record), then `status`. Original backups remain intact.

Version matching applies even when two versions have identical DLLs. The current
RC record must remain with its RC recipe and tools. The v1.1.0 recipe
refuses it; `--record` does not bypass that check. Keep the installed RC unchanged
until a separate transition is approved. Never change a record's release or hash.

For sealed v1.0.3 or any different pair, the new installer deliberately refuses
those DLLs as unknown. Before migration, with the game closed and writers idle:

1. Make a durable backup outside the game and TEMP of the existing proxy, payload,
   marker and loose bank, recording exact relative paths, SHA-256, sizes and which
   files were absent. Separately preserve `playlist.ini`, Music (including external
   paths), Cache, State, unrelated files and any `.Supermoon-Recovery` records.
2. Verify known v1.0.3 bytes against the sealed release's trusted checksums. A foreign
   proxy/bank or ambiguous ownership is a stop condition; retain it and investigate.
3. Resolve an older active recovery record through step 4 first, if present. After
   verification, quarantine only the individually identified old Supermoon proxy,
   payload, marker and generated bank outside the game. Keep their original
   relative-path map and all backups. Do not delete/rename whole Supermoon or MODS
   directories, personal data, native banks or unrelated mods.
4. If an older active recovery record exists, use that exact version's trusted tool
   to inspect/disable it and verify the managed files are absent first. Preserve
   and verify a full durable copy of its `.Supermoon-Recovery` tree, then move that
   old recovery tree out of the game to the durable migration backup. The old
   selector remains part of that backup; the new installer needs a fresh recovery
   root and cannot read/overwrite a different-version active record. Never edit
   records to change their release. If the old tool cannot disable/verify its record,
   stop for review before quarantining or installing anything.
5. Run the clean installation procedure below from the new staging directory.
   Existing personal files remain. A new record backs up the resulting absence,
   not the quarantined historical pair: cross-version automatic rollback is not supplied.

Do not blindly reactivate a quarantined pair: it requires its matching
bank, supported executable and separate qualification. Disable the candidate to
return to absence; keep historical bytes available for an explicitly reviewed restore.

## Clean install after approval

1. Keep the downloaded/extracted package in a persistent staging directory
   **outside the game tree and TEMP**. Verify the v1.1.0 archive against the
   accompanying release `SHA256SUMS` obtained through the owner's trusted release
   channel. Until publication is approved, only local prepared artifacts exist.
   A self-consistent manifest is not a signature. The old private ZIP hashes do
   not identify the public packages; never relabel those ZIPs or mix policies.
   Never merge a package over the game as the installation/update procedure.
2. Check the supported executable identity in `build/recipe.json` and
   `docs/BUILD_AND_PROVENANCE.md`. The tool independently verifies the exact
   executable, package DLL pair/marker, native archive, inner bank and generated
   output. A game update requires a newly qualified policy; do not bypass refusal.
3. With NMS closed and exclusive access established, use the trusted v1.1.0 tool:

```powershell
$gameDir = 'D:\SteamLibrary\steamapps\common\No Man''s Sky'
$stageDir = 'D:\Mod Staging\Supermoon'
$setup = Join-Path $stageDir 'Supermoon\tools\install_supermoon.py'
python --version
python $setup status --game $gameDir
python $setup install --game $gameDir --package $stageDir
python $setup status --game $gameDir
```

`$stageDir` contains `winmm.dll`, the activation marker and `Supermoon/`.
Use the same canonical game path for later operations. The installation tool
does not load DLLs or execute NMS. The bank is derived locally from your game;
neither the native archive nor any native game file is replaced.

Before changing a target, the tool creates
`<game>/.Supermoon-Recovery/records/<record-id>/`. It saves and reads back prior
and replacement bytes, an immutable hash-checked manifest, game/release/DLL/bank
identities and prior absence/presence. An atomic `ACTIVE.json` selector is published
only after these records verify. Copies and records are flushed to storage.
Recovery storage is outside TEMP by policy; an installation under TEMP is refused.
Keep the printed record location, all prior records and the trusted setup tools.
Copy recovery storage outside the game before uninstalling/moving the game.

Activation's marker is removed first, the proxy/payload/bank are updated as needed,
and the marker is restored last. Each target is rehashed. A mutable journal records
prepared, in-progress, failed and verified stages. This is **not an atomic four-file
transaction**. A failure can leave a complete new bank and incomplete other work.
The tool reports that possibility and preserves recovery data instead of attempting
an unsafe automatic rollback. Filesystem/directory persistence through power loss
is not established by flush/readback or process-termination tests.

Repeated `install` on an unfinished/current record validates and reuses its original
prior state. It does not replace the first backup with the newly installed bank.
After an explicit completed rollback/disable, a later install creates a new record
and retains the old one. Unknown existing proxy/payload/marker/bank bytes must first
be preserved and investigated by the operator; there is no force-overwrite option.


Successful installation reports JSON `result: verified` and a durable record
location; status reports actual identities separately from the journal. A refusal
may report `REFUSED_OR_INCOMPLETE`; files may already have changed. Use status and
the recovery procedure below. Phase 6 verified the actual candidate installation
and recovery record; these labels alone do not prove audible operation or which
bank NMS loads.

For read-only identity checks (compare DLL hashes with `build/recipe.json` or
`RELEASE_DLL_SHA256SUMS` in the matching source package):

```powershell
Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $gameDir 'Binaries\NMS.exe')
Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $stageDir 'winmm.dll')
Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $stageDir 'Supermoon\supermoon_playlist.dll')
```

Only Steam build 26385139 is targeted: NMS.exe is 89,535,560 bytes with SHA-256
`a77fc29877789c3da9181afeb5941b9b85a27d3df24919a24403cf8e194c6109`.
The required generated bank is 36,017 bytes, SHA-256
`65bcbdd3cd9aaae513d21865e504f194afba584ee012f9d3d558e9c04f71a54f`.
Generation is mandatory; do not download a native/generated bank from another user.
The four-file install command derives and installs it without modifying the native
archive. `prepare_audio.py --game $gameDir` alone verifies derivation in memory.

## Personal configuration and desktop registration

The new package builder supplies `playlist.example.ini`, never a live `playlist.ini`.
The installer does not copy either into a personal configuration. A clean install
uses runtime defaults: `Music` relative to Supermoon and a 3072 MiB cache budget.
If desired, copy the example to `playlist.ini` **only when no personal INI exists**,
then edit it deliberately while the game is closed. Preserve existing music paths,
normalization, keyboard settings, cache budget, Cache and State during updates.
Installation preserves these files; normal runtime cache maintenance can still
remove obsolete managed entries after a complete scan. Original music is separate.

Add personal supported WAV/MP3 files to `Binaries/Supermoon/Music` unless your
existing INI selects another directory. Top-level files only; no music is shipped.
Retain original licenses/notices with binary redistribution.

Ordinary desktop launches require no new arguments. Use NMS's normal mod settings
to register/enable the required bank entry; the tools do not edit settings or
fabricate entries. First-time registration may require normal exit/restart after
NMS saves its settings. See [Phase 2A](docs/REMEDIATION_PHASE2A.md) for admission
and VR limitations. An enabled entry and valid disk bank do not prove effective
resource selection or audible behavior. Resolve actual required-bank conflicts;
unrelated mods are not blanket-prohibited.

VR is unsupported and not reliably detected. Other stores/builds and Vortex/MO2/
other mod-manager deployment are unqualified. Manual staging is the documented
path. Known enabled loose init.bnk conflicts refuse; packed/legacy resource
priority is not fully inspected. Keep incompatible overrides of that resource out
of the supported stack, while retaining unrelated mods. Never infer effective
loading from a hash or an enabled entry alone.

The Discoveries workaround changes ByteBeat volume in the Frontend state,
including native tracks. It persists independently of DLL activation. Suppression
is based on custom carrier intent through silence/EOF, but M2 prevents a guarantee
of native soundtrack restoration after every Stop. H1's isolated cleanup self-wait
has unproven gameplay reachability. Both are explicitly owner-accepted unresolved
limitations; original severities, failed tests and audit classifications remain.
Normal Stop restoration and menu behavior passed Phase 6 by owner report, not an
all-schedule proof. Lifecycle capacity is cumulative
and finite; manual Play after native Stop/travel/reload remains necessary.

## Diagnose, retry, roll back or uninstall (disable)

Keep NMS closed and retain exclusive access. Use the trusted tool, not a legacy
TEMP receipt or files of unknown origin:

```powershell
python $setup status --game $gameDir
python $setup resume --game $gameDir
python $setup rollback --game $gameDir
# Alternative: remove only recognized program resources, preserving personal data.
python $setup disable --game $gameDir
```

Choose the action deliberately; these are alternatives, not a sequence to run blindly.

- `status` is read only. It reports actual file identities separately from the
  recorded journal, prior/candidate/absent/UNKNOWN classifications, record IDs and
  matching scratch candidates. A candidate bank present on disk is not proof that
  the full operation completed, even if the last journal write failed.
- `resume` verifies immutable backups and the supported executable, then completes
  the saved candidate without requiring the original package or native archive.
  It preserves the first prior-state manifest. A missing/corrupt journal is not
  trusted as proof of completion; actual files are checked again.
- `rollback` verifies **all** current targets and backups before changing any target.
  It restores the exact prior state, including absence, only for an all-absent
  clean prior state or a known matching prior DLL/marker pair with the current
  qualified bank (or an absent bank/inactive prior setup). It refuses incomplete
  or unqualified prior combinations. The accepted historical menu-only bank may
  be backed up during an update, but is not qualified for automatic reactivation.
- `disable` removes only managed files whose current identities are recognized
  by this record. It keeps personal files and every backup. After an incompatible
  NMS update, use disable rather than force old DLLs/banks to activate. Unknown
  current files still block the operation; they are not treated as ours to delete.

A corrupt active selector or a fully prepared but unselected record can be
inspected explicitly, using the 32-hex ID reported in the recovery directory:

```powershell
python $setup status --game $gameDir --record '<record-id>'
python $setup resume --game $gameDir --record '<record-id>'
```

The explicit record still must pass schema, game-path, release, identity and backup
verification. It does not authorize arbitrary paths or unknown bytes. Records
without complete verified manifests were interrupted **before target replacement**;
preserve them for diagnosis. Do not edit JSON/hashes to make a damaged record pass.
If a backup is damaged, retrieve a separately verified copy of the same record.
If current files are unknown, preserve and hash them outside the game and establish
their ownership before deliberately quarantining them. Only then can absence be
repaired from verified bytes. Never silently overwrite a foreign proxy or bank.

Termination immediately before replacement can leave a `.supermoon-<id>-*.tmp`
file. Status lists candidates but never deletes unknown scratch automatically.
Keep records first; remove a scratch file manually only after checking its exact
location, recorded ID and candidate identity with NMS closed/exclusive access.
Do not delete whole Supermoon/MODS directories as cleanup. Recovery does not remove
native archives, executable files, settings, saves, personal data or unrelated mods.

The compatibility command `prepare_audio.py --game <game> --install` now uses the
same durable record mechanism, and requires the exact DLL pair **and marker** to
already be present. It cannot recover what an earlier manual DLL extraction
overwrote. Prefer the four-file installer so its first backup precedes all changes.
Without `--install`, preparation remains generation in memory with no game writes.

Hot unload is unsupported. Any activation refusal after possible native hook
exposure requires normal process exit before another attempt. Installation and
recovery success do not resolve M2/H1 or expand the finite Phase 6 qualification.

For complete removal or after an incompatible game update, disable BOTH proxy
activation and this verified loose bank. Removing only the DLL leaves the bank's
Frontend effect enabled. `disable` preserves user files and recovery storage;
leave empty directories and unrelated mod resources alone. The tool is not a
general-purpose uninstaller and does not remove package documentation or personal data.
