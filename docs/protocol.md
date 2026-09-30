# Protocol specification

This is experimental draft 0.1. MUST, MUST NOT, SHOULD, and MAY express normative
requirements. YAML registries own numeric identifiers; this document owns their
semantics. Implementations MUST apply both. [Framing](framing.md) defines every byte.

## Message namespace

| Range         | Purpose                                                    |
| ------------- | ---------------------------------------------------------- |
| 0x0000–0x00ff | Handshake, errors, future capability updates; 0 is invalid |
| 0x0100–0x01ff | Events; event ID plus 0x0100                               |
| 0x0200–0x02ff | Queue commands; command IDs 1–8 plus 0x0200                |
| 0x0300–0x03ff | Inspection; command IDs 9–11 plus 0x0300                   |
| 0x0400–0x04ff | Responses; command ID plus 0x0400                          |
| 0x0500–0x05ff | Reserved heartbeat/status                                  |
| 0x0600–0xffff | Reserved                                                   |

Only the six message types below have encodings in this draft. Reserved IDs MUST
NOT be sent or accepted as implemented messages. Published IDs are never reused.

## Implemented schemas

Field names refer to the table below. All fields not listed are forbidden if
known; unknown optional tags follow the framing extension rule.

| Type                 | ID     | Required fields                         | Optional fields        | Sequence |
| -------------------- | ------ | --------------------------------------- | ---------------------- | -------- |
| HELLO                | 0x0001 | min_version, max_version, capabilities  | none                   | 0        |
| HELLO_ACK            | 0x0002 | selected_version, capabilities          | none                   | 0        |
| ERROR                | 0x0003 | error_code                              | request_id, error_text | 0        |
| MESSAGE_ACCEPTED     | 0x0102 | server_id, exim_id, event_id, timestamp | tracking_id, sender    | nonzero  |
| QUEUE_COUNT          | 0x0202 | request_id                              | none                   | 0        |
| QUEUE_COUNT_RESPONSE | 0x0402 | request_id, queue_count                 | none                   | 0        |

## Fields

| Tag | Name             | Wire type        | Constraint                                       |
| --- | ---------------- | ---------------- | ------------------------------------------------ |
| 1   | min_version      | 2 uint8          | major, minor                                     |
| 2   | max_version      | 2 uint8          | same major as min; max minor >= min minor        |
| 3   | selected_version | 2 uint8          | must be within both advertised ranges            |
| 4   | capabilities     | uint64           | independent capability bits                      |
| 5   | request_id       | uint64           | nonzero; scoped to connection                    |
| 6   | server_id        | UTF-8 identifier | 1–64 bytes                                       |
| 7   | tracking_id      | UTF-8 identifier | 1–128 bytes                                      |
| 8   | exim_id          | UTF-8 identifier | 1–64 bytes                                       |
| 9   | recipient_id     | UTF-8 identifier | 1–128 bytes; reserved, currently forbidden       |
| 10  | event_id         | UTF-8 identifier | 1–128 bytes                                      |
| 11  | timestamp        | uint64           | microseconds since Unix epoch, UTC; zero allowed |
| 12  | queue_count      | uint64           | zero allowed                                     |
| 13  | error_code       | uint16           | registered non-OK error code                     |
| 14  | error_text       | UTF-8 text       | 0–1024 bytes                                     |
| 15  | sender           | UTF-8 text       | 0–320 bytes; empty denotes null reverse path     |

All lengths are encoded byte lengths, not Unicode code points. Identifiers use
the ASCII subset `[A-Za-z0-9._:@+-]`; this makes exact byte comparison portable.
No Unicode normalization, case folding, silent replacement, or truncation occurs.
Text accepts valid Unicode scalar values encoded with shortest-form UTF-8;
embedded NUL, surrogates, overlong sequences, and values above U+10FFFF are invalid.
Missing optional fields differ from present empty strings. Required identifiers
cannot be empty. Transport adapters MUST NOT assume NUL termination.

Future field limits are reserved design constraints, not implemented fields:
recipient 320, hostname 253, textual IP address 45, router 128, transport 128,
SMTP response 1024 bytes; event data stays within the 65,536-byte payload cap.
Future schemas must specify content validation (such as IP syntax) as well as
these limits before implementation. This draft does not accept these fields
merely because a maximum is documented.

## Error codes

See [registry](registries.md) for exact stable numeric values. OK is a local API
success result, never a valid ERROR payload. INVALID_FRAME, UNSUPPORTED_VERSION,
UNSUPPORTED_MESSAGE, UNSUPPORTED_CAPABILITY, INVALID_REQUEST, INVALID_MESSAGE_ID,
INVALID_TRACKING_ID, MESSAGE_NOT_FOUND, MESSAGE_BUSY, PERMISSION_DENIED,
INTERNAL_ERROR, and TEMPORARY_ERROR are machine-readable outcomes.

Malformed field syntax returns INVALID_REQUEST in the reference codecs.
INVALID_MESSAGE_ID and INVALID_TRACKING_ID are reserved for higher-level
semantic rejection of otherwise well-formed IDs. Busy/not-found/permission and
internal/temporary errors describe peer outcomes; the codec does not implement
the operations producing them. Human-readable error_text is optional and clients
MUST NOT branch on its wording. Unknown error enum values fail this draft's
validation; future codes require negotiated support or a known fallback code.

## API boundary

C: `exob_encode`, `exob_decode`, `exob_validate`, `exob_negotiate`.
Functions return numeric registry error codes. Decode failure leaves no usable
partial frame. See the public header for buffer ownership and output rules.

Go: `protocol.Encode`, `protocol.Decode`, `protocol.Validate`, `protocol.Negotiate`
from `github.com/inode64/exim-observer-protocol/go`. Failures use typed `Code`
errors. Decode borrows field values from the input; Encode returns owned bytes.

Codec validation is stateless. Applications must additionally check handshake
state, the selected header version, capability gates, request correlation,
durable event identity, peer identity and authorization. No helper here opens a
socket, executes a command, maintains session state, or stores a WAL.
