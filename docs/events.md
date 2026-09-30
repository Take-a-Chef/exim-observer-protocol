# Events

`spec/events.yaml` reserves MESSAGE_RECEIVED, MESSAGE_ACCEPTED, MESSAGE_REJECTED,
RECIPIENT_ADDED, DELIVERY_START, DELIVERY_SUCCESS, DELIVERY_DEFER, DELIVERY_FAIL,
MESSAGE_COMPLETE, MESSAGE_FROZEN, MESSAGE_THAWED, MESSAGE_REMOVED, BOUNCE_RECEIVED,
DSN_RECEIVED, OPEN, and CLICK. Only MESSAGE_ACCEPTED has a payload schema today.
All other event IDs are placeholders and MUST NOT be emitted under this draft.

MESSAGE_ACCEPTED records a message accepted for processing, not a delivery
success. It requires the events capability and a nonzero header sequence,
server_id, exim_id, event_id, and timestamp. Optional tracking_id requires tracking;
optional sender can be empty for the SMTP null reverse path. See
[protocol fields](protocol.md) for exact tags and lengths.

Sequence is scoped to a durable logical server_id, not a process, agent, or
connection. A producer must serialize allocation across processes for that server,
start at 1, increase monotonically for new events, and preserve values on replay.
Restart must not reset the counter. If durable identity/counter state is lost,
provision a new server_id before emitting. uint64 exhaustion also requires a new
server_id; wrapping is forbidden. Server identifiers must distinguish independent
producer domains. The protocol does not implement allocation or persistence.

Receivers deduplicate by `(server_id, sequence)`. event_id is a producer-assigned
stable event reference for correlation and must remain unchanged on replay; it
is not an alternative deduplication key. Reusing an identity for different content
is a protocol violation. A replay uses the original frame values, including time.
Out-of-order arrival can occur during replay; do not discard every sequence below
the largest seen. Track received identities or contiguous durable watermarks.

Sequence gaps permit loss detection but do not prove permanent loss. These
semantics support future WAL acknowledgement and replay control messages; this
draft has no acknowledgement/replay request message and promises neither
exactly-once delivery nor an event delivery transport.
