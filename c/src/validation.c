#include "exob_protocol.h"
#include <stdbool.h>

struct field_rule {
    uint16_t id, max;
    uint8_t kind;
};
struct message_rule {
    uint16_t type;
    uint32_t required, allowed;
};
#include "registry.inc"

static bool valid_text(const uint8_t *v, size_t n) {
    size_t i = 0;
    while (i < n) {
        uint8_t c = v[i++];
        if (c == 0) {
            return false;
        }
        if (c < 0x80) {
            continue;
        }
        uint32_t cp = 0, min = 0;
        size_t count = 0;
        if (c >= 0xc2 && c <= 0xdf) {
            cp = c & 0x1fu;
            min = 0x80;
            count = 1;
        } else if (c >= 0xe0 && c <= 0xef) {
            cp = c & 0x0fu;
            min = 0x800;
            count = 2;
        } else if (c >= 0xf0 && c <= 0xf4) {
            cp = c & 0x07u;
            min = 0x10000;
            count = 3;
        } else {
            return false;
        }
        if (count > n - i) {
            return false;
        }
        for (size_t j = 0; j < count; ++j) {
            uint8_t next = v[i++];
            if ((next & 0xc0u) != 0x80u) {
                return false;
            }
            cp = (cp << 6u) | (next & 0x3fu);
        }
        if (cp < min || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) {
            return false;
        }
    }
    return true;
}
static bool valid_identifier(const uint8_t *v, size_t n) {
    if (n == 0) {
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        uint8_t c = v[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == ':' || c == '@' || c == '+' || c == '-')) {
            return false;
        }
    }
    return true;
}
static bool valid_field(const exob_field *f, const struct field_rule *r) {
    if (f->length > r->max) {
        return false;
    }
    switch (r->kind) {
    case 0:
    case 1:
        return f->length == r->max;
    case 2:
        return f->length == 8 && exob_read_uint(f->value, 8) != 0;
    case 3:
        return valid_identifier(f->value, f->length);
    case 4:
        return valid_text(f->value, f->length);
    case 5:
        return f->length == 2 && exob_read_uint(f->value, 2) > EXOB_ERROR_OK &&
               exob_read_uint(f->value, 2) <= EXOB_ERROR_TEMPORARY_ERROR;
    default:
        return false;
    }
}
static bool valid_range(exob_range r) {
    return r.min.major == r.max.major && r.min.minor <= r.max.minor;
}
uint16_t exob_validate(const exob_frame *frame) {
    if (frame == NULL) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    if (frame->version.major != EXOB_MAJOR || frame->version.minor < EXOB_MINOR) {
        return EXOB_ERROR_UNSUPPORTED_VERSION;
    }
    if (frame->flags != 0 || frame->field_count > EXOB_MAX_FIELDS) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    const struct message_rule *rule = NULL;
    for (size_t i = 0; i < sizeof(message_rules) / sizeof(message_rules[0]); ++i) {
        if (message_rules[i].type == frame->type) {
            rule = &message_rules[i];
            break;
        }
    }
    if (rule == NULL) {
        return EXOB_ERROR_UNSUPPORTED_MESSAGE;
    }
    if ((frame->type == EXOB_MSG_MESSAGE_ACCEPTED) != (frame->sequence != 0)) {
        return EXOB_ERROR_INVALID_REQUEST;
    }
    uint32_t seen = 0;
    uint16_t prev = 0;
    size_t size = 0;
    for (size_t i = 0; i < frame->field_count; ++i) {
        const exob_field *f = &frame->fields[i];
        if (f->id == 0 || f->id <= prev || (f->value == NULL && f->length > 0)) {
            return EXOB_ERROR_INVALID_FRAME;
        }
        prev = f->id;
        if ((size_t)f->length + 4u > EXOB_MAX_PAYLOAD - size) {
            return EXOB_ERROR_INVALID_FRAME;
        }
        size += 4u + f->length;
        const struct field_rule *r = NULL;
        for (size_t j = 0; j < sizeof(field_rules) / sizeof(field_rules[0]); ++j) {
            if (field_rules[j].id == f->id) {
                r = &field_rules[j];
                break;
            }
        }
        if (r == NULL) {
            if ((f->id & 0x8000u) != 0) {
                return EXOB_ERROR_INVALID_REQUEST;
            }
            continue;
        }
        uint32_t bit = UINT32_C(1) << f->id;
        if ((rule->allowed & bit) == 0 || !valid_field(f, r)) {
            return EXOB_ERROR_INVALID_REQUEST;
        }
        seen |= bit;
    }
    if ((seen & rule->required) != rule->required) {
        return EXOB_ERROR_INVALID_REQUEST;
    }
    if (frame->type == EXOB_MSG_HELLO) {
        const uint8_t *a = frame->fields[0].value, *b = frame->fields[1].value;
        exob_range range = {{a[0], a[1]}, {b[0], b[1]}};
        if (!valid_range(range)) {
            return EXOB_ERROR_INVALID_REQUEST;
        }
    }
    return EXOB_ERROR_OK;
}
uint16_t exob_negotiate(exob_range a, exob_range b, uint64_t a_caps, uint64_t b_caps,
                        exob_version *selected, uint64_t *caps) {
    if (selected == NULL || caps == NULL) {
        return EXOB_ERROR_INVALID_REQUEST;
    }
    *selected = (exob_version){0, 0};
    *caps = 0;
    if (!valid_range(a) || !valid_range(b) || a.min.major != b.min.major) {
        return EXOB_ERROR_UNSUPPORTED_VERSION;
    }
    uint8_t lo = a.min.minor > b.min.minor ? a.min.minor : b.min.minor;
    uint8_t hi = a.max.minor < b.max.minor ? a.max.minor : b.max.minor;
    if (lo > hi) {
        return EXOB_ERROR_UNSUPPORTED_VERSION;
    }
    *selected = (exob_version){a.min.major, hi};
    *caps = a_caps & b_caps;
    return EXOB_ERROR_OK;
}
