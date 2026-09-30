# Code generation

Status: Accepted for experimental draft 0.1.

## Context

Numeric constants, validation masks, and documentation can drift from the specification.

## Decision

Small Python/PyYAML development tooling validates spec/*.yaml and generates C constants, validation rules, and Markdown registry tables. Commit these artifacts.

## Alternatives

Manual duplication is simple initially but risks interoperability; generating entire parsers hides language-specific safety review.

## Consequences

Codec parsing remains hand-written. make check-generated compares regenerated temporary files without editing the tree, including untracked files. No runtime dependencies are added. Python, PyYAML, clang-format, and pinned Prettier are explicit developer dependencies.
