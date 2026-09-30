# Wire format

Status: Accepted for experimental draft 0.1.

## Context

We need deterministic bounded framing, optional evolution, and a simple C parser that other implementations can independently reproduce.

## Decision

Use a fixed 24-byte EXOB header and ordered uint16 tag/length/value payload, capped at 64 KiB and 32 fields. Unknown optional tags survive round trips; unknown critical tags fail.

## Alternatives

Native structs are nonportable. JSON adds numeric/canonicalization issues; a serialization framework adds dependencies before requirements justify one. A variable header adds parsing states.

## Consequences

Explicit byte access handles unaligned input. Large body/list operations require a future bounded chunking design. The field API is generic but generated schemas enforce message-specific requirements.
