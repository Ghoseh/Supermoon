# Compatibility maintenance

Version 1.0.1 targets NMS Steam build 26271400. The behavior reference is
Supermoon v1.0.0, commit `d5ff3ca`. This update adapts native integration without
changing playlist, preparation, control or recovery behavior.

Use the pinned toolchain and portable commands in BUILD_AND_PROVENANCE.md.
MIGRATION_STATUS.md records the adaptation and performed checks.

Private executable comparisons, native fixtures, raw reviewer/test receipts and
runtime captures are retained outside the public repository. They are not required
to compile the mod and must not be added to public source archives.

The updated pair is built, reproduced and live-tested. Existing accepted findings
remain documented. A new four-hour endurance test has not been performed.
