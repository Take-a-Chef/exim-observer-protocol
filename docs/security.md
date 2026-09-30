# Protocol security

The primary transport is a local Unix socket between the plugin and agent.
Local input is still untrusted. Parsers are a security boundary and must treat
lengths, flags, identifiers, enums, UTF-8, timestamps, and all optional data as
attacker-controlled. Bounded byte parsing must precede use of a field.

Deployment integrations enforce socket ownership/mode, peer credentials, least
privilege, and operation-specific authorization. Negotiating queue-control is not
permission to mutate a queue. The wire format itself provides no authentication,
encryption, integrity protection, or access-control implementation. Any future
remote transport needs an independently specified authenticated secure channel.

Never allocate an unbounded peer-declared payload; enforce the 64 KiB cap before
reading/allocating it. Enforce TLV count and per-field bounds. Partial frame timeout,
connection quotas, rate limits, and backpressure are transport responsibilities.
Do not resynchronize after bad framing. Avoid reflecting unvalidated text or IDs
in errors. Render text safely in its eventual log/UI context; valid UTF-8 does
not make a string safe as HTML, shell input, a path, or a database query.

Message body/header/log access and tracking can expose personal or sensitive
content. Capabilities must be independently gated and authorized. Error messages
should not reveal private message data. Secrets and production captures do not
belong in test vectors or fuzz corpora.

C decode performs no heap allocation; field pointers borrow input and the caller
provides storage for the bounded field descriptors. Callers must preserve buffer
lifetime and avoid mutation while using a decoded frame. C encode buffers must
not overlap the frame's source values. Failure outputs are not partially usable.
Sanitizers, shared negative fixtures, static analysis, and fuzzing check these
invariants; none replace review of new schemas and parsers.
