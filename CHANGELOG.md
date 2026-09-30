# Changelog

Repository releases use Semantic Versioning; wire compatibility is tracked separately.

## [Unreleased]

### Added

- Experimental wire draft 0.1 with explicit framing, negotiation, one event and one control operation.
- Portable C11 codec with bounded validation.
- Shared golden, malformed, compatibility, round-trip, and fuzz tests.
- Authoritative registries, generated constants, developer tooling, and GitHub Actions.

### Changed

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
