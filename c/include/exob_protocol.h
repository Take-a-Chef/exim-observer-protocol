#ifndef EXOB_PROTOCOL_INTERNAL_H
#define EXOB_PROTOCOL_INTERNAL_H
/* Internal conveniences; consumers need only include/exim_observer_protocol.h. */
#include "exim_observer_protocol.h"
/* Call only after the containing slice has been bounds checked. */
static inline uint64_t exob_read_uint(const uint8_t *p, size_t n) {
    uint64_t v = 0;
    for (size_t i = 0; i < n; ++i) {
        v = (v << 8u) | p[i];
    }
    return v;
}
static inline void exob_write_uint(uint8_t *p, size_t n, uint64_t v) {
    for (size_t i = n; i > 0; --i) {
        p[i - 1] = (uint8_t)(v & UINT64_C(255));
        v >>= 8u;
    }
}
#endif
