# Capability and version negotiation

Capabilities are a big-endian uint64 bitmap. Bit positions 0–14 are registered
in `spec/capabilities.yaml`: events, tracking, queue-list, queue-control,
message-headers, message-body, message-log, force-delivery, freeze, thaw, remove,
bounce, dsn, engagement, and queue-count. Bits 15–63 are reserved for additions.
Unknown bits are retained; they never authorize unknown behavior. A peer MUST
advertise only functionality it implements and is willing to expose.

## Handshake state machine

1. On a new connection the initiator sends exactly one HELLO using the bootstrap
   header version 0.1. No application messages may precede negotiation.
2. HELLO carries a minimum and maximum version in one major and an offered bitmap.
   The responder intersects that range with its supported range and selects the
   highest minor in the overlap. A range spanning majors is invalid. A product
   supporting multiple majors chooses one major per attempt; automatic retry is
   integration policy, not a codec responsibility.
3. With overlap, the responder sends HELLO_ACK using bootstrap header 0.1,
   selected_version, and the intersection of offered and locally enabled bits.
   It need not echo its entire supported range. The initiator verifies that the
   selection is within its offer and the ACK bitmap is a subset of its offer.
4. No overlap produces ERROR/UNSUPPORTED_VERSION using bootstrap 0.1 and closes
   the connection. A syntactically valid HELLO advertising 1.x is decodable by the
   draft codec but incompatible with a responder supporting only draft 0.1.
5. Subsequent messages MUST use the selected header version and negotiated
   capabilities. Duplicate handshakes and unsolicited ACKs are INVALID_REQUEST.
   Renegotiation requires a new connection in this draft.

Only range 0.1–0.1 is implemented for live sessions here. Generic intersection
tests with 1.x/2.x demonstrate the algorithm, not implemented future wire support.
The standalone decoder tolerates minor >= 1 for major 0 to prove optional-field
compatibility, but a session MUST reject a minor other than its selected version.
Bootstrap framing belongs to this major family; a future major must define its
own bootstrap compatibility story.

## Operation gates

| Operation/data                          | Required negotiated bits |
| --------------------------------------- | ------------------------ |
| MESSAGE_ACCEPTED                        | events                   |
| tracking_id on an event                 | events and tracking      |
| QUEUE_COUNT and its successful response | queue-count              |
| ERROR / handshake                       | none                     |

An endpoint sending or accepting an operation without its gate returns
UNSUPPORTED_CAPABILITY when a valid request can be correlated. A capability is
necessary but never sufficient for authorization. Tracking does not imply events;
queue-control does not imply freeze, thaw, remove, or force-delivery. Future
mutations will require queue-control plus their individual operation bit.
Inspection bits and engagement/bounce/DSN bits do not enable reserved messages in
this draft. Capability updates and heartbeat messages are reserved, not implemented.

`Negotiate` / `exob_negotiate` return the raw intersection, including shared unknown
bits, and do not validate an ACK or authorize operations. This separation keeps
the protocol foundation independent of agent/server session implementations.
