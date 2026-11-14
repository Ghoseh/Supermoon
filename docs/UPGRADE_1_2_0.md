# Upgrade public v1.2.0 to v1.2.1

Close NMS, back up personal files/records, verify and extract the complete v1.2.1
ZIP outside the game/TEMP. Run Setup.cmd, confirm the game root, choose **8**
and type **UPGRADE**. Ordinary Update intentionally refuses a cross-version pair.

The upgrade accepts only the exact published v1.2.0 DLLs on Steam build 26540391:

| DLL | Bytes | SHA-256 |
| --- | ---: | --- |
| winmm.dll | 948224 | e2ff82fec494f62549a9de6e94fee90a07292170d57ec1b94e8e2cbf94533b56 |
| supermoon_playlist.dll | 6165504 | 0bb13ce24c6adaf351245ce95fcecfd4373e43c0ef9b17531ced23e15543d2fe |

Matching selector, manifests and saved backups are verified. A fully recognized
already-disabled installation is also accepted. Unknown/modified DLLs, private
prototype policies, corrupt/missing records and unsupported executable builds
refuse. Existing inactive records must not be deleted to force fresh Install.

The procedure validates the new package before disabling anything, uses matching
public v1.2.0 Disable code under the installation writer lock, and verifies all
four managed files absent. It leaves original records in `.Supermoon-Recovery`,
copies them into `.Supermoon-Recovery-1.2.1/archives`, and verifies every copy.
New v1.2.1 records then capture all-absent prior state before installing the pair.
Require verified completion and Status candidate_present/game_supported true.

Music, Cache, State, logs, saves and unrelated mods are preserved. Existing INI
values/comments are retained; setup backs up the original and adds only missing
settings so all 12 options are available to adjust. Missing
VoiceGainDb now defaults to 3 dB; explicit existing values are not changed.
The bank remains the same locally generated supported bank. No old DLL is a
new-record rollback target. Keep both record stores and the matching packages.

## If interrupted

Keep NMS closed and retain all files. Run matching v1.2.1 Status. Choose Resume
with the complete package, or repeat Upgrade; completed phases are verified
before proceeding. If a new v1.2.1 record exists, recovery uses only that record.
Before a new record exists, verified old Disable can finish reaching absence.

To stop safely, choose Disable (or inactive Rollback). It removes only recognized
resources; it never restores old DLLs across the version boundary. Verify all
four managed paths absent. A failed Disable/rollback must not be ignored: resolve
the reported sharing/ownership issue, then retry with matching tools. On corrupt,
unknown or inconsistent state, preserve both stores and request support. Do not
manually edit records, swap DLLs, relabel policies or restore incompatible pairs.
