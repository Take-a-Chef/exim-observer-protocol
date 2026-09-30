# Framing

Draft 0.1 uses the following 24-byte header. All multibyte integers, including
TLV tags and lengths, are unsigned big-endian (network order). No padding exists.

| Offset | Bytes | Field          | Encoding                     |
| ------ | ----- | -------------- | ---------------------------- |
| 0      | 4     | magic          | ASCII `EXOB`, hex `45584f42` |
| 4      | 1     | protocol_major | uint8                        |
| 5      | 1     | protocol_minor | uint8                        |
| 6      | 2     | message_type   | uint16                       |
| 8      | 4     | flags          | uint32; MUST be zero         |
| 12     | 4     | payload_length | uint32; excludes header      |
| 16     | 8     | sequence       | uint64                       |

The offsets align naturally for common integer widths, but decoders MUST use
byte operations and MUST NOT cast a wire buffer to a native struct. Input may
start at any address. C `sizeof`, pointer sizes, enum layout, host endian, and
compiler packing are irrelevant to the wire contract.

## Payload

A payload is a sequence of TLVs: `tag:uint16`, `length:uint16`, then exactly
`length` value bytes. Tags MUST be nonzero, unique, and strictly increasing by
unsigned numeric value. No alignment, terminator, or padding follows a value.
Integers have the exact width prescribed in the message schema. Versions are two
uint8 values, major followed by minor. Empty payloads fail the initial schemas.

Tags `0x0001–0x7fff` are optional-to-understand. A tag unknown to the implementation
is preserved and ignored semantically. Tags `0x8000–0xffff` are critical-to-understand;
unknown critical tags reject the message with INVALID_REQUEST. The high bit is
part of the tag identity, not masked off. Known fields can still be mandatory for
a message: missing them is invalid. Known fields forbidden in that message are
also invalid. At most 32 fields, including unknown fields, are permitted.

Canonical encoders emit the caller's already ordered fields and reject noncanonical
objects. Unknown optional values are opaque bytes, not assumed to be UTF-8.
Preserving them permits `encode(decode(bytes)) == bytes`.

## Bounds and failure behavior

Maximum payload: **65,536 bytes**. Maximum complete frame: **65,560 bytes**.
Maximum single TLV value: **65,535 bytes**, further restricted by its field schema
and total payload size. Compare declared length to the cap before allocation or
addition. Check remaining bytes before each read. No length-derived allocation
is needed in C. Decode stores at most 32 field descriptors in the caller-provided
frame and borrows values from the input. Encode writes to a caller-owned bounded buffer.

Decode takes exactly one complete frame. Short input, trailing bytes, bad magic,
nonzero flags, duplicate/unsorted tags, truncated TLVs, and excessive lengths are
INVALID_FRAME. Unknown message types are UNSUPPORTED_MESSAGE; unsupported header
versions are UNSUPPORTED_VERSION. Field schema failures are INVALID_REQUEST.
Validation precedence is structural frame/TLV parsing, version, flags/count,
message type, sequence, then field schema. Multiply-invalid input has no promised
single error classification; implementations must agree on the published fixtures.

A stream integration reads the fixed header first, rejects excessive length, and
then reads the bounded payload. Stream buffering, timeouts, and I/O are outside
these codecs. On framing failure close the connection; do not scan for a new
magic value. For a structurally valid bad request, a session may return ERROR
with the validated request_id. Never trust a correlation ID extracted from a
malformed frame. Decoder failure returns no usable partial object.

## Extensibility review

A fixed header avoids a second variable-length parser. Reserved flags do not
implicitly enable extensions. Compatible additions use optional TLVs and negotiated
capabilities; header-layout changes need a new major. The 64 KiB cap bounds parser
work and memory and is ample for this milestone. Message bodies and large queue
listings need a future bounded chunking design; they cannot bypass this cap.
