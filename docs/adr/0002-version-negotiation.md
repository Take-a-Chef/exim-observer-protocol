# Version negotiation

Status: Accepted for experimental draft 0.1.

## Context

Software versions do not identify wire compatibility or feature support.

## Decision

Use separate uint8 major/minor ranges within one major and independent uint64 capability intersection. Bootstrap draft handshake uses 0.1; subsequent headers must match selection.

## Alternatives

Inferring capabilities from minor versions conflates syntax and enabled features. Cross-major ranges need substantially more selection rules.

## Consequences

Each handshake offers one major. Generic negotiation examples do not imply codec support for those future versions. Session enforcement remains outside this stateless codec library.
