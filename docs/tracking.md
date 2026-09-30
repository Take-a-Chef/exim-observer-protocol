# Tracking and correlation

tracking_id optionally groups events belonging to an external tracking context.
It is an opaque, case-sensitive 1–128-byte identifier, not a URL or an instruction.
It requires negotiated tracking support and never replaces exim_id, the local
message identifier. Neither identifier is assumed globally unique alone.

server_id names a durable producer identity. Header sequence plus server_id is
the authoritative replay/deduplication identity. event_id provides a stable
cross-system reference to that individual event. recipient_id is reserved for
future per-recipient events and is forbidden in the initial message schemas.
request_id correlates commands and responses within one connection and is not
an event sequence. See events.md for sequence durability and replay rules.

OPEN and CLICK identifiers are reserved only. This repository does not implement
tracking pixels, redirect endpoints, recipient analytics, or event detection.
Future engagement payloads need separate privacy and capability review.
