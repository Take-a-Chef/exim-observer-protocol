# Compatibility contract

Within a stable major, numeric IDs and field meanings never change or get reused.
Removed identifiers remain reserved. Required behavior changes trigger major
version consideration. Add optional data through new optional tags and negotiate
new capabilities. A new required field or enum value is not automatically compatible.

Unknown optional fields must survive decode/re-encode without changing bytes.
Unknown critical fields, flags, message types, and enum values fail explicitly.
Reusing an already known field on a new message is capability/version gated if
older schemas would forbid that combination. Do not assume all schema extensions
are ignorable merely because their tag has the high bit clear.

The compatibility contract is `testdata/`: C tests encode semantic fixture objects
to canonical binary and decode binary to expected objects. A Python-generated
test-only initializer adapter supplies the expectations independently of the codec.
Byte changes or semantic drift fail the golden tests.

Other implementations can consume these language-neutral fixtures from separate
repositories. Cross-language integration testing belongs with those consumers;
this repository tests C conformance, version negotiation, and capability intersection.

`testdata/compatibility/negotiation.json` covers overlap, disjoint majors, disjoint
minors, reversed ranges, ranges crossing majors, and empty capability intersection.
The newer-minor optional-extension vector tests preserving unknown data. The
incompatible HELLO is valid syntax; negotiation must return UNSUPPORTED_VERSION.

Protocol changes must update registries, prose, the C implementation, golden and
malformed fixtures, compatibility tests, and changelog together. Stable versioned
vectors are append-only. CI checks generated content without mutating the tree.
