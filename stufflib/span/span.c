#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/context/context.h>
#include <stufflib/iterator/iterator.h>
#include <stufflib/macros/macros.h>
#include <stufflib/memory/memory.h>
#include <stufflib/span/span.h>

struct sl_span sl_span_view(size_t size, unsigned char data[size]) {
  assert(size);
  return (struct sl_span){
      .owned = false,
      .size  = size,
      .data  = data,
  };
}

bool sl_span_create(struct sl_context ctx[static 1], size_t size, struct sl_span out[static 1]) {
  if (!size) {
    *out = (struct sl_span){.owned = true, .size = 0, .data = nullptr};
    return true;
  }
  void* data = sl_alloc(ctx, size, sizeof(unsigned char));
  if (!data) {
    SL_ERROR(ctx, "failed allocating span of size %zu", size);
    return false;
  }
  *out = (struct sl_span){.owned = true, .size = size, .data = data};
  return true;
}

void sl_span_destroy(struct sl_span data[static 1]) {
  if (data->owned && data->data) {
    sl_free(data->data);
  }
  *data = (struct sl_span){0};
}

void sl_span_clear(struct sl_span data[const static 1]) {
  memset(data->data, 0, data->size);
}

bool sl_span_copy(
    struct sl_context ctx[static 1],
    struct sl_span src[const static 1],
    struct sl_span out[static 1]
) {
  if (!sl_span_create(ctx, src->size, out)) {
    return false;
  }
  if (out->data && out->size) {
    memcpy(out->data, src->data, out->size);
  }
  return true;
}

bool sl_span_from_str(
    struct sl_context ctx[static 1],
    const char str[const static 1],
    struct sl_span out[static 1]
) {
  size_t num_chars = strlen(str);
  if (!sl_span_create(ctx, num_chars, out)) {
    return false;
  }
  if (out->data && out->size) {
    memcpy(out->data, str, out->size);
  }
  return true;
}

bool sl_span_is_hexadecimal_str(struct sl_span src[const static 1]) {
  return src->size > 2 && memcmp(src->data, "0x", 2) == 0;
}

bool sl_span_parse_hex(
    struct sl_context ctx[static 1],
    struct sl_span src[const static 1],
    struct sl_span out[static 1]
) {
  assert(sl_span_is_hexadecimal_str(src));
  size_t num_bytes = ((src->size - 2) + 1) / 2;
  if (!sl_span_create(ctx, num_bytes, out)) {
    return false;
  }
  for (size_t i_byte = 0; i_byte < out->size; ++i_byte) {
    size_t i1   = (2 * i_byte) + 2;
    size_t i2   = (2 * i_byte) + 3;
    char byte[] = {
        (char)(src->data[i1]),
        (char)(i2 < src->size ? src->data[i2] : 0),
        0,
    };
    out->data[i_byte] = (unsigned char)strtoul(byte, 0, 16);
  }
  return true;
}

bool sl_span_concat(
    struct sl_context ctx[static 1],
    struct sl_span data1[const static 1],
    struct sl_span data2[const static 1],
    struct sl_span out[static 1]
) {
  if (!sl_span_create(ctx, data1->size + data2->size, out)) {
    return false;
  }
  if (data1->size) {
    memcpy(out->data, data1->data, data1->size);
  }
  if (data2->size) {
    memcpy(out->data + data1->size, data2->data, data2->size);
  }
  return true;
}

bool sl_span_extend(
    struct sl_context ctx[static 1],
    struct sl_span dst[static 1],
    struct sl_span src[const static 1]
) {
  struct sl_span tmp = {0};
  if (!sl_span_concat(ctx, dst, src, &tmp)) {
    return false;
  }
  sl_span_destroy(dst);
  *dst = tmp;
  return true;
}

struct sl_span sl_span_slice(struct sl_span data[const static 1], size_t begin, size_t end_or_max) {
  size_t end = SL_MIN(end_or_max, data->size);
  if (begin >= end) {
    return (struct sl_span){0};
  }
  size_t slice_size          = end - begin;
  unsigned char* slice_begin = data->data + begin;
  return sl_span_view(slice_size, slice_begin);
}

struct sl_span
sl_span_find(struct sl_span data[const static 1], struct sl_span pattern[const static 1]) {
  if (data->size && pattern->size) {
    for (size_t begin = 0; begin + pattern->size <= data->size; ++begin) {
      unsigned char* subseq = data->data + begin;
      if (memcmp(subseq, pattern->data, pattern->size) == 0) {
        size_t subseq_size = data->size - begin;
        return sl_span_view(subseq_size, subseq);
      }
    }
  }
  return (struct sl_span){0};
}

int sl_span_compare(struct sl_span lhs[const static 1], struct sl_span rhs[const static 1]) {
  if (lhs->data && rhs->data) {
    return memcmp(lhs->data, rhs->data, SL_MIN(lhs->size, rhs->size));
  }
  return (!rhs->data) - (!lhs->data);
}

void* sl_span_iter_get(struct sl_iterator iter[const static 1]) {
  struct sl_span* data = iter->data;
  return data->data + iter->index;
}

void sl_span_iter_advance(struct sl_iterator iter[const static 1]) {
  ++(iter->index);
  ++(iter->pos);
}

bool sl_span_iter_is_done(struct sl_iterator iter[const static 1]) {
  struct sl_span* data = iter->data;
  return iter->index == data->size;
}

struct sl_iterator sl_span_iter(struct sl_span data[const static 1]) {
  return (struct sl_iterator){.data = (void*)data};
}
