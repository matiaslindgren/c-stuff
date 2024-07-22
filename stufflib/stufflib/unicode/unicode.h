#ifndef SL_UNICODE_H_INCLUDED
#define SL_UNICODE_H_INCLUDED
// Definition of well-formed UTF-8 sequences taken from the Unicode standard:
// "The Unicode® Standard Version 15.0" – Core Specification
// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#G7404
// accessed 2023-02-18
#include <stdint.h>

#include "../iterator/iterator.h"
#include "../span/span.h"

static const uint32_t sl_unicode_error_value = UINT32_MAX;
static const size_t sl_unicode_error_width = 0;

size_t sl_unicode_codepoint_width(uint32_t value);

size_t sl_unicode_codepoint_width_from_utf8(size_t size,
                                            unsigned char bytes[const size]);

uint32_t sl_unicode_codepoint_from_utf8(size_t width,
                                        unsigned char bytes[const width]);

bool sl_unicode_is_valid_utf8(struct sl_span data[const static 1]);

size_t sl_unicode_iter_item_width(struct sl_iterator iter[const static 1]);

void sl_unicode_iter_advance(struct sl_iterator iter[const static 1]);

bool sl_unicode_iter_is_done(struct sl_iterator iter[const static 1]);

void* sl_unicode_iter_get(struct sl_iterator iter[const static 1]);

uint32_t sl_unicode_iter_decode_item(struct sl_iterator iter[const static 1]);

struct sl_iterator sl_unicode_iter(struct sl_span data[const static 1]);

size_t sl_unicode_length(struct sl_span data[const static 1]);

#endif  // SL_UNICODE_H_INCLUDED
