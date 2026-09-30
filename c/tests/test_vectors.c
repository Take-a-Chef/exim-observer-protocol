#include "exim_observer_protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
struct vector {
    const char *name;
    const uint8_t *wire;
    size_t length;
    uint16_t error;
    exob_frame semantic;
};
struct negotiation {
    uint64_t a_caps, b_caps, caps;
    uint16_t error;
    exob_version selected;
    exob_range a, b;
};
#include "vectors.inc"
static void same_frame(const exob_frame *a, const exob_frame *b) {
    assert(a->version.major == b->version.major && a->version.minor == b->version.minor);
    assert(a->type == b->type && a->flags == b->flags && a->sequence == b->sequence);
    assert(a->field_count == b->field_count);
    for (size_t i = 0; i < a->field_count; ++i) {
        assert(a->fields[i].id == b->fields[i].id && a->fields[i].length == b->fields[i].length);
        assert(memcmp(a->fields[i].value, b->fields[i].value, a->fields[i].length) == 0);
    }
}
int main(void) {
    uint8_t encoded[EXOB_HEADER_SIZE + EXOB_MAX_PAYLOAD];
    for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
        const struct vector *v = &vectors[i];
        exob_frame decoded;
        uint16_t code = exob_decode(v->wire, v->length, &decoded);
        if (code != v->error) {
            fprintf(stderr, "%s: got %u expected %u\n", v->name, (unsigned)code,
                    (unsigned)v->error);
            return 1;
        }
        if (code != EXOB_ERROR_OK) {
            assert(decoded.field_count == 0);
            continue;
        }
        same_frame(&decoded, &v->semantic);
        size_t written = 0;
        assert(exob_encode(&v->semantic, encoded, sizeof(encoded), &written) == EXOB_ERROR_OK);
        assert(written == v->length && memcmp(encoded, v->wire, written) == 0);
        assert(exob_encode(&decoded, encoded, sizeof(encoded), &written) == EXOB_ERROR_OK);
        assert(written == v->length && memcmp(encoded, v->wire, written) == 0);
        exob_frame roundtrip;
        assert(exob_decode(encoded, written, &roundtrip) == EXOB_ERROR_OK);
        same_frame(&roundtrip, &v->semantic);
        for (size_t n = 0; n < v->length; ++n) {
            assert(exob_decode(v->wire, n, &roundtrip) != EXOB_ERROR_OK);
        }
    }
    for (size_t i = 0; i < sizeof(negotiations) / sizeof(negotiations[0]); ++i) {
        const struct negotiation *n = &negotiations[i];
        exob_version v;
        uint64_t caps = 0;
        assert(exob_negotiate(n->a, n->b, n->a_caps, n->b_caps, &v, &caps) == n->error);
        assert(v.major == n->selected.major && v.minor == n->selected.minor && caps == n->caps);
    }
    puts("C shared golden, malformed, round-trip and negotiation vectors passed");
    return 0;
}
