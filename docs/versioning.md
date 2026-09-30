# Versioning

Wire versions are two unsigned 8-bit integers. A major changes for incompatible
header layouts, mandatory semantics, or behaviors that cannot be negotiated.
A minor adds backward-compatible optional functionality within one major.
Neither counter wraps; exhaustion requires an explicit replacement design.

Software releases use their own semantic version. plugin_version, agent_version,
server_version, and protocol_version are distinct concepts. No software-version
fields are encoded in this milestone. A plugin 0.7.2 supporting wire 1.2 and an
agent 1.3.0 supporting wire 1.0–1.4 can select 1.2 regardless of software versions.

Draft 0.1 is EXPERIMENTAL; no backward-compatibility guarantee exists before the
first stable protocol release. Draft breaking changes still require visible
changelog entries, review, new versioned vectors, and a documented migration.
Never silently rewrite a released stable golden vector.

The C API and wire protocol have independent stability policies. Stable
wire bytes do not freeze every helper API. Release notes must identify both
separately. Consumer APIs in other repositories follow their own release policies. Git tags describe repository software releases; they do not select a
wire version. Releasing stable 1.0 requires an explicit stabilization review.
