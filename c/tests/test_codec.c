#include "exim_observer_protocol.h"
#include <assert.h>
#include <string.h>
int main(void) {
    exob_frame frame = {0};
    assert(exob_decode(NULL, 0, &frame) == EXOB_ERROR_INVALID_FRAME);
    assert(exob_decode(NULL, 0, NULL) == EXOB_ERROR_INVALID_FRAME);
    uint8_t request[] = {0, 0, 0, 0, 0, 0, 0, 1};
    frame.version = (exob_version){EXOB_MAJOR, EXOB_MINOR};
    frame.type = EXOB_MSG_QUEUE_COUNT;
    frame.field_count = 1;
    frame.fields[0] = (exob_field){EXOB_FIELD_REQUEST_ID, 8, request};
    uint8_t out[36];
    size_t written = 99;
    assert(exob_encode(&frame, out, sizeof(out) - 1, &written) == EXOB_ERROR_INVALID_FRAME &&
           written == 0);
    assert(exob_encode(&frame, NULL, sizeof(out), &written) == EXOB_ERROR_INVALID_FRAME);
    assert(exob_encode(&frame, out, sizeof(out), NULL) == EXOB_ERROR_INVALID_FRAME);
    assert(exob_encode(&frame, out, sizeof(out), &written) == EXOB_ERROR_OK &&
           written == sizeof(out));
    exob_frame decoded;
    for (size_t i = 0; i < 1000; ++i) {
        assert(exob_decode(out, written, &decoded) == EXOB_ERROR_OK);
        assert(decoded.fields[0].value == out + 28);
    }
    frame.fields[0].value = NULL;
    assert(exob_validate(&frame) == EXOB_ERROR_INVALID_FRAME);
    assert(exob_validate(NULL) == EXOB_ERROR_INVALID_FRAME);
    return 0;
}
