#ifndef EXIM_OBSERVER_PROTOCOL_H
#define EXIM_OBSERVER_PROTOCOL_H
#include "exob_registry.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Host API structures, NEVER wire layouts. No function allocates memory.
 * Decoded field values borrow the input buffer; keep it alive and unmodified.
 * Output frame is zeroed on decode failure. Encode input/output must not overlap.
 * All functions return EXOB_ERROR_OK or a registry error code. */
typedef struct {
    uint8_t major, minor;
} exob_version;
typedef struct {
    exob_version min, max;
} exob_range;
typedef struct {
    uint16_t id, length;
    const uint8_t *value;
} exob_field;
typedef struct {
    exob_version version;
    uint16_t type;
    uint32_t flags;
    uint64_t sequence;
    size_t field_count;
    exob_field fields[EXOB_MAX_FIELDS];
} exob_frame;
uint16_t exob_validate(const exob_frame *frame);
uint16_t exob_decode(const uint8_t *input, size_t length, exob_frame *frame);
/* written is zero on failure. Caller provides capacity; no truncation. */
uint16_t exob_encode(const exob_frame *frame, uint8_t *output, size_t capacity, size_t *written);
/* Pure intersection; this does not authorize operations or mutate a session. */
uint16_t exob_negotiate(exob_range a, exob_range b, uint64_t a_caps, uint64_t b_caps,
                        exob_version *selected, uint64_t *caps);
#ifdef __cplusplus
}
#endif
#endif
