# Changelog

Repository releases use Semantic Versioning; wire compatibility is tracked separately.

## [Unreleased]

### Added

- Pinned Go analysis tools and CI checks adapted from fsledger: golangci-lint,
  NilAway, govulncheck and Go formatters; security/correctness rules include tests.

- Standard-library Go codec, generated registries, shared golden/negative vectors,
  negotiation tests and native fuzzing; wire draft remains unchanged.

- Experimental wire draft 0.1 with explicit framing, negotiation, one event and one control operation.
- Portable C11 codec with bounded validation.
- Shared golden, malformed, compatibility, round-trip, and fuzz tests.
- Authoritative registries, generated constants, developer tooling, and GitHub Actions.

### Changed

- Require Go 1.27.0 and test the 1.27.x series; add explicit modernization and
  read-only `go fix -diff` checks without changing the wire protocol.

- Scoped the implementation, tooling, and CI exclusively to C. Observer code belongs in a separate project.

### Deprecated

- None.

### Removed

- Initial Go package, module, tests, fuzz target, code generation, and development/CI dependencies.

### Fixed

- None.

### Security

- Parser size bounds and validation are security-boundary requirements.

**Protocol-breaking changes:** this initial experimental draft establishes new bytes;
no stable compatibility promise exists. Future breaking changes must be listed here explicitly.
