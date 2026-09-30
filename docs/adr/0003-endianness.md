# Endianness

Status: Accepted for experimental draft 0.1.

## Context

Every implementation must produce identical bytes on every host architecture.

## Decision

All multibyte integers are big-endian, including TLV tags, lengths, counts, sequence, and capability bits.

## Alternatives

Host endian is not portable; little-endian is workable but offers no compelling protocol advantage here.

## Consequences

Use explicit reads/writes, never packed structs or pointer casts. Byte-oriented fixtures test exact values in C and provide a contract for external implementations.
