# Distribution privacy and portability

The selected public files include production source, pinned generated inputs,
portable build/test tooling, dependency notices, public summaries and exact release
DLLs in the separate installation ZIP. This is an explicit selection, not a copy
of the private development tree. See PUBLIC_FILE_MANIFEST.json and SHA256SUMS.

Music, caches, State, game settings/saves, personal filenames, PDBs, raw command
receipts, diagnostic captures, historical audit archives, compiler archives and
rollback materials are excluded. No secret or developer-specific absolute path may
be shipped. Required third-party copyright names and contact addresses remain.

Runtime Music defaults to the Music child of the mod directory, derived from the
game executable location, independently of the shell's working directory. Relative
overrides resolve against the mod directory. Cache, State and playlist.ini are also
mod-relative. System WINMM uses Windows system-directory APIs. Absolute Music
overrides are optional user choices; public examples use Music=Music.

Final checks inspect every selected file/archive member, names and metadata,
ASCII/UTF-16 binary strings, PE debug/COFF/resource metadata, credentials patterns,
profile/absolute paths, account/machine/IP identifiers and unexpected extras.
Production game/profile RVAs and hashes are interoperability data, not credentials.
No whole game binary, private game fixture or SDK is included. Short pinned native
instruction profiles required by integration do not grant rights to the game.

Pattern scans cannot prove absence of every arbitrary name or encoded secret.
Review unexplained hits and preserve required attribution; recheck exact hashes
after any change. A clean input review does not clear a later modified ZIP.
Raw runtime logs may reveal paths and filenames: review/redact excerpts before
submitting an issue. No public telemetry upload or hosted account is required.
