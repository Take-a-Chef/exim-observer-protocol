# Test vector format

Status: Accepted for experimental draft 0.1.

## Context

The C implementation needs independent semantic expectations and exact reference bytes without production parser dependencies. Other repositories must be able to consume the same fixtures.

## Decision

Store descriptive JSON companions and binary files. JSON includes semantic input, expected decoded object, numeric error, and binary hex. C test adapters are generated from these objects into ignored build files.

## Alternatives

Adding a C JSON/YAML parser to production is unnecessary. Binary-only fixtures cannot independently verify field semantics. JSON keeps fixture metadata readable by standard tooling in many languages.

## Consequences

Python development tooling checks pairs and builds C test initializers. Future consumers implement their own fixture readers in their repositories. Fixture updates are explicit via tools/create-vectors.py, never part of normal generation. Stable released fixtures remain immutable.
