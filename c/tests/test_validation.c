#include "exim_observer_protocol.h"
#include <assert.h>
int main(void) {
    uint8_t code[] = {0, 1};
    const uint8_t unicode[] = {0x7f, 0xc2, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80, 0xed, 0x9f,
                               0xbf, 0xf0, 0x90, 0x80, 0x80, 0xf4, 0x8f, 0xbf, 0xbf};
    exob_frame f = {0};
    f.version = (exob_version){EXOB_MAJOR, EXOB_MINOR};
    f.type = EXOB_MSG_ERROR;
    f.field_count = 2;
    f.fields[0] = (exob_field){EXOB_FIELD_ERROR_CODE, 2, code};
    f.fields[1] = (exob_field){EXOB_FIELD_ERROR_TEXT, sizeof(unicode), unicode};
    assert(exob_validate(&f) == EXOB_ERROR_OK);
    for (unsigned i = 0; i < 256; ++i) {
        uint8_t byte = (uint8_t)i;
        f.fields[1] = (exob_field){EXOB_FIELD_ERROR_TEXT, 1, &byte};
        assert((exob_validate(&f) == EXOB_ERROR_OK) == (i > 0 && i < 128));
    }
    f.field_count = EXOB_MAX_FIELDS + 1;
    assert(exob_validate(&f) == EXOB_ERROR_INVALID_FRAME);
    exob_range r = {{0, 1}, {0, 1}};
    uint64_t caps = 0;
    assert(exob_negotiate(r, r, 0, 0, NULL, &caps) == EXOB_ERROR_INVALID_REQUEST);
    return 0;
}
