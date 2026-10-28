# Compatibility repair handoff

Worktree branch: `update/nms-26271400`. Original release branch remains unchanged.

Use the pinned Zig 0.15.2 compiler documented in BUILD_AND_PROVENANCE.md. `python -B tools/build.py --zig <zig.exe>` reproduces the pinned updated DLL pair in disposable output. `python -B tools/run_tests.py --zig <zig.exe>` runs the portable regression selection.

Private native comparison/generation scripts, binary-input fixtures, reviewer receipts, build and test receipts, installation/backup manifest and pending live evidence are in the owner's separate `Supermoon-Hook-Audit-20261028` directory. They are not public release inputs.

See MIGRATION_STATUS.md for the adaptation matrix and actual evidence. Current status: built, offline tested, installed and live accepted. Owner confirmed playback, automatic advancement, teleport/warp recovery and playback after relaunch. First session exited with code 0. Warm activation took 0.651 seconds. No new four-hour endurance qualification is claimed.
