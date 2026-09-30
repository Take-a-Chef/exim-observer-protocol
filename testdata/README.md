# Shared wire contract

`valid/` and `invalid/` pair descriptive `draft01-*.json` files with exact `.bin`
bytes. Each companion contains name, protocol_version, semantic input,
expected_decoded (null for errors), expected_error (numeric Code), and binary_hex.
Integer field bytes appear as hex to avoid JSON uint64 precision differences;
headers and fixture control values are bounded JSON integers. The normative type
and interpretation of each field is in docs/protocol.md. Invalid semantic input
can describe an impossible object; binary_hex is authoritative for malformed bytes.

The valid incompatible-range HELLO is syntactically valid, but a draft-only peer
cannot negotiate it. It belongs in valid/, with no-overlap negotiation covered
separately in `compatibility/negotiation.json`. No network or clock is consulted.

C tests encode semantic input to the binary golden file and decode
that binary to the expected fields. They also decode/re-encode canonical bytes.
`tools/vectors.py` verifies all pairs and makes the C semantic initializer adapter
under build/. It never derives expectations from the codec being tested.
The language-neutral files can also be consumed by implementations in other
repositories; this repository does not run cross-language integration tests.

Run `make test-vectors` and `make test-compat`. Update experimental goldens only
through an intentional reviewed change (the initial recipe is
`python3 tools/create-vectors.py`, followed by `make fmt`). `make generate` does
not rewrite fixtures. Do not run the initial recipe to overwrite stable releases;
create new versioned fixtures instead. Existing stable goldens are immutable.

The corpus covers all implemented messages, unknown optional fields, newer minor
syntax, limits, wrong magic/version/type/flags, truncation, TLV overrun, duplicate
fields, missing fields, invalid UTF-8/enums/identifiers, and uint32 overflow attempts.
SMTP responses and other future payloads have no wire schema yet; add their
malformed vectors when those schemas are implemented.
