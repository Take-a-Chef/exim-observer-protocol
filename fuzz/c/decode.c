#include "exim_observer_protocol.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    exob_frame frame;
    if (exob_decode(data, size, &frame) == EXOB_ERROR_OK) {
        uint8_t out[EXOB_HEADER_SIZE + EXOB_MAX_PAYLOAD];
        size_t written = 0;
        if (exob_validate(&frame) != EXOB_ERROR_OK ||
            exob_encode(&frame, out, sizeof(out), &written) != EXOB_ERROR_OK || written != size ||
            memcmp(out, data, size) != 0) {
            abort();
        }
    }
    return 0;
}
