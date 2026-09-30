#include "exob_protocol.h"
#include <string.h>

uint16_t exob_decode(const uint8_t *input, size_t length, exob_frame *frame) {
    if (frame == NULL) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    memset(frame, 0, sizeof(*frame));
    if (input == NULL || length < EXOB_HEADER_SIZE || memcmp(input, "EXOB", 4) != 0) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    uint64_t payload = exob_read_uint(input + 12, 4);
    if (payload > EXOB_MAX_PAYLOAD || payload != length - EXOB_HEADER_SIZE) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    exob_frame tmp = {0};
    tmp.version.major = input[4];
    tmp.version.minor = input[5];
    tmp.type = (uint16_t)exob_read_uint(input + 6, 2);
    tmp.flags = (uint32_t)exob_read_uint(input + 8, 4);
    tmp.sequence = exob_read_uint(input + 16, 8);
    size_t pos = EXOB_HEADER_SIZE;
    while (pos < length) {
        if (length - pos < 4 || tmp.field_count == EXOB_MAX_FIELDS) {
            return EXOB_ERROR_INVALID_FRAME;
        }
        exob_field *field = &tmp.fields[tmp.field_count];
        field->id = (uint16_t)exob_read_uint(input + pos, 2);
        field->length = (uint16_t)exob_read_uint(input + pos + 2, 2);
        pos += 4;
        if (field->length > length - pos) {
            return EXOB_ERROR_INVALID_FRAME;
        }
        field->value = input + pos;
        pos += field->length;
        ++tmp.field_count;
    }
    uint16_t result = exob_validate(&tmp);
    if (result == EXOB_ERROR_OK) {
        *frame = tmp;
    }
    return result;
}

uint16_t exob_encode(const exob_frame *frame, uint8_t *output, size_t capacity, size_t *written) {
    if (written == NULL) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    *written = 0;
    uint16_t result = exob_validate(frame);
    if (result != EXOB_ERROR_OK) {
        return result;
    }
    size_t size = EXOB_HEADER_SIZE;
    for (size_t i = 0; i < frame->field_count; ++i) {
        size += 4u + frame->fields[i].length;
    }
    if (output == NULL || capacity < size) {
        return EXOB_ERROR_INVALID_FRAME;
    }
    memcpy(output, "EXOB", 4);
    output[4] = frame->version.major;
    output[5] = frame->version.minor;
    exob_write_uint(output + 6, 2, frame->type);
    exob_write_uint(output + 8, 4, frame->flags);
    exob_write_uint(output + 12, 4, size - EXOB_HEADER_SIZE);
    exob_write_uint(output + 16, 8, frame->sequence);
    size_t pos = EXOB_HEADER_SIZE;
    for (size_t i = 0; i < frame->field_count; ++i) {
        const exob_field *f = &frame->fields[i];
        exob_write_uint(output + pos, 2, f->id);
        exob_write_uint(output + pos + 2, 2, f->length);
        if (f->length > 0) {
            memcpy(output + pos + 4, f->value, f->length);
        }
        pos += 4u + f->length;
    }
    *written = size;
    return EXOB_ERROR_OK;
}
