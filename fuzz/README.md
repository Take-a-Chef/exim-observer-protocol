# Fuzzing

`make fuzz-c` builds a Clang libFuzzer target with ASan/UBSan. It uses the valid
and invalid binary fixtures as seeds, copying them into ignored build/corpus
before mutation. `make fuzz-smoke` runs this C target for about ten seconds;
longer local runs can use the commands in Makefile.

One decoder target reaches framing, every initial event/control/handshake schema,
and field validation. Successful decode must validate and re-encode to exactly the
same bytes within the size cap. Failed decode must not crash, overrun, hang, leak,
or return a usable partial object. Minimize crash artifacts and convert them into
named testdata vectors for regression coverage. Never commit private traffic.

The Go decoder fuzz target lives in `go/codec_test.go` to share the Go package
and the canonical C/Go corpus. Run `make fuzz-go`; successful decodes must
re-encode byte for byte, including unknown optional fields.
