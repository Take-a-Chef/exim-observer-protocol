# Security policy

No stable version has been released. Security fixes target the current development
branch; old experimental drafts have no support guarantee. A stable release must
publish its supported-version matrix before claiming ongoing maintenance.

Parsers are part of the security boundary. Malformed protocol input, out-of-bounds
access, unbounded resource use, and cross-language validation discrepancies are
security-sensitive even when transport is a local Unix socket.

Use GitHub's private vulnerability reporting for this repository when it is enabled:
<https://github.com/inode64/exim-observer-protocol/security/advisories/new>.
If unavailable, contact an inode64 maintainer through the contact information on
their GitHub profile and request a private channel. Do not publish exploit details
in a public issue. This skeleton does not establish a response-time commitment.

Include affected revision/version, compiler/runtime, minimal non-sensitive input,
reproduction commands, sanitizer output, and expected versus observed behavior.
See [protocol security](docs/security.md) for integration responsibilities.
