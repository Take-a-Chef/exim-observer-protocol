# Exim Observer Protocol

Open, versioned protocol for Exim Observer event tracking, queue control, message monitoring, and service interoperability.

**Protocol status: EXPERIMENTAL. Current draft: 0.1. No stable wire release exists.**
The C and Go APIs are also experimental and versioned separately from the wire protocol.

This repository is the language-neutral specification, compatibility contract,
shared test-vector corpus, and C/Go reference codecs for:

```text
Exim ↔ exim-observer-plugin (C) ↔ Exim Observer Protocol ↔ exim-observer-agent (Go)
```

The C plugin and Go agent use the respective codecs from this repository. The
agent implementation lives in a separate project. Other implementations can use the specification
and test vectors without depending on C memory layouts. Integers have explicit widths and
big-endian encodings; native structure layouts are never transmitted.

The first milestone implements framing, HELLO, HELLO_ACK, ERROR,
MESSAGE_ACCEPTED, QUEUE_COUNT, QUEUE_COUNT_RESPONSE, and pure version/capability
negotiation. Future events and commands have reserved identifiers only.
It contains no Exim event detection, queue execution, database, agent/server,
HTTP, WebUI, or Prometheus implementation.

## Start here

- [Protocol and message schemas](docs/protocol.md)
- [Framing and bounds](docs/framing.md)
- [Negotiation](docs/capabilities.md) and [compatibility](docs/compatibility.md)
- [Generated numeric registries](docs/registries.md)
- [Shared fixtures](testdata/README.md) and [decisions](docs/adr/README.md)
- [Development setup](CONTRIBUTING.md)

With a C11 compiler, Go 1.27.x and Python 3 installed, normal tests are local and need no network:

```sh
make test
make test-vectors
make test-compat
```

After installing the documented development tools:

```sh
make fmt
make check
make test-sanitize CC=clang
make fuzz-smoke
```

`make check` enforces formatting, registries, generated files, static analysis,
unit tests, golden vectors, and compatibility. CI adds GCC/Clang, sanitizers,
libFuzzer smoke tests, and CodeQL for C. CI results must be
checked on GitHub; the presence of workflows does not establish a green run.

## Consumers and layout

C consumers include `include/exim_observer_protocol.h` (and its generated registry
header) and compile `c/src/codec.c` and `c/src/validation.c` with `-Iinclude -Ic/include`.
The API allocates no memory; decoded values borrow caller-owned input buffers.

Go consumers import `github.com/Take-a-Chef/exim-observer-protocol/go` (package
`protocol`). `Encode`, `Decode`, `Validate` and `Negotiate` use only the standard
library. Decode borrows input bytes; Encode returns an owned buffer. C and Go
consume the exact same JSON/binary fixtures, including malformed inputs.

The layout includes C and Go implementations and their tests. `tools/` contains development-only generators;
`include/exob_registry.h`, `c/src/registry.inc`, `go/registry.go`, and
`docs/registries.md` are generated. `build/vectors.inc` adapts shared JSON semantics
for C tests only. JSON companions provide language-neutral semantic expectations
for both implementations and future consumers. These decisions are recorded in the ADRs.

## Related projects

- [exim-observer-plugin](https://github.com/Take-a-Chef/exim-observer-plugin)
- [exim-observer-agent](https://github.com/Take-a-Chef/exim-observer-agent)
- [exim-observer](https://github.com/Take-a-Chef/exim-observer)

Licensed under [Apache-2.0](LICENSE).
