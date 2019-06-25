#include <stddef.h>
#include <stdlib.h>
#include "sysdep.h"
#include "constants.h"
#include <limits.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct erlpack_buffer {
  char *buf;
  size_t length;
  size_t allocated_size;
} erlpack_buffer;

static inline int erlpack_buffer_write(erlpack_buffer *pk, const char *bytes,
                                       size_t l) {
  char *buf = pk->buf;
  size_t allocated_size = pk->allocated_size;
  size_t length = pk->length;

  if (length + l > allocated_size) {
    // Grow buffer 2x to avoid excessive re-allocations.
    allocated_size = (length + l) * 2;
    buf = (char *)realloc(buf, allocated_size);

    if (!buf)
      return -1;
  }

  memcpy(buf + length, bytes, l);
  length += l;

  pk->buf = buf;
  pk->allocated_size = allocated_size;
  pk->length = length;
  return 0;
}

#define erlpack_append(pk, buf, len)                                           \
  return erlpack_buffer_write(pk, (const char *)buf, len)

static inline int erlpack_append_version(erlpack_buffer *b) {
  static unsigned char buf[1] = {FORMAT_VERSION};
  erlpack_append(b, buf, 1);
}

static inline int erlpack_append_nil(erlpack_buffer *b) {
  static unsigned char buf[5] = {SMALL_ATOM_EXT, 3, 'n', 'i', 'l'};
  erlpack_append(b, buf, 5);
}
static inline int erlpack_append_false(erlpack_buffer *b) {
  static unsigned char buf[7] = {SMALL_ATOM_EXT, 5, 'f', 'a', 'l', 's', 'e'};
  erlpack_append(b, buf, 7);
}

static inline int erlpack_append_true(erlpack_buffer *b) {
  static unsigned char buf[6] = {SMALL_ATOM_EXT, 4, 't', 'r', 'u', 'e'};
  erlpack_append(b, buf, 6);
}

static inline int erlpack_append_small_integer(erlpack_buffer *b,
                                               unsigned char d) {
  unsigned char buf[2] = {SMALL_INTEGER_EXT, d};
  erlpack_append(b, buf, 2);
}

static inline int erlpack_append_integer(erlpack_buffer *b, int32_t d) {
  unsigned char buf[5];
  buf[0] = INTEGER_EXT;
  _erlpack_store32(buf + 1, d);
  erlpack_append(b, buf, 5);
}

static inline int erlpack_append_unsigned_long_long(erlpack_buffer *b,
                                                    unsigned long long d) {
  unsigned char buf[1 + 2 + sizeof(unsigned long long)];
  buf[0] = SMALL_BIG_EXT;

  unsigned char bytes_enc = 0;
  while (d > 0) {
    buf[3 + bytes_enc] = d & 0xFF;
    d >>= 8;
    bytes_enc++;
  }
  buf[1] = bytes_enc;
  buf[2] = 0;

  erlpack_append(b, buf, 1 + 2 + bytes_enc);
}

static inline int erlpack_append_long_long(erlpack_buffer *b, long long d) {
  unsigned char buf[1 + 2 + sizeof(unsigned long long)];
  buf[0] = SMALL_BIG_EXT;
  buf[2] = d < 0 ? 1 : 0;
  unsigned long long ull = d < 0 ? -d : d;
  unsigned char bytes_enc = 0;
  while (ull > 0) {
    buf[3 + bytes_enc] = ull & 0xFF;
    ull >>= 8;
    bytes_enc++;
  }
  buf[1] = bytes_enc;
  erlpack_append(b, buf, 1 + 2 + bytes_enc);
}

typedef union {
  uint64_t ui64;
  double df;
} typePunner;

static inline int erlpack_append_double(erlpack_buffer *b, double f) {
  unsigned char buf[1 + 8] = {0};
  buf[0] = NEW_FLOAT_EXT;
  typePunner p;
  p.df = f;
  _erlpack_store64(buf + 1, p.ui64);
  erlpack_append(b, buf, 1 + 8);
}

static inline int erlpack_append_atom(erlpack_buffer *b, const char *bytes, size_t size) {
  if (size < 255) {
    unsigned char buf[2] = {SMALL_ATOM_EXT, (unsigned char)size};
    int ret = erlpack_buffer_write(b, (const char *)buf, 2);
    if (ret < 0)
      return ret;

    erlpack_append(b, bytes, size);
  } else {
    unsigned char buf[3];
    buf[0] = ATOM_EXT;

    if (size > 0xFFFF) {
      return 1;
    }

    _erlpack_store16(buf + 1, size);

    int ret = erlpack_buffer_write(b, (const char *)buf, 3);
    if (ret < 0)
      return ret;

    erlpack_append(b, bytes, size);
  }
}

static inline int erlpack_append_atom_utf8(erlpack_buffer *b, const char *bytes, size_t size) {
  if (size < 255) {
    unsigned char buf[2] = {SMALL_ATOM_UTF8_EXT, (unsigned char)size};
    int ret = erlpack_buffer_write(b, (const char *)buf, 2);
    if (ret < 0)
      return ret;

    erlpack_append(b, bytes, size);
  } else {
    unsigned char buf[3];
    buf[0] = ATOM_UTF8_EXT;

    if (size > 0xFFFF) {
      return 1;
    }

    _erlpack_store16(buf + 1, size);

    int ret = erlpack_buffer_write(b, (const char *)buf, 3);
    if (ret < 0)
      return ret;

    erlpack_append(b, bytes, size);
  }
}

static inline int erlpack_append_binary(erlpack_buffer *b, const char *bytes, size_t size) {
  unsigned char buf[5];
  buf[0] = BINARY_EXT;

  _erlpack_store32(buf + 1, size);

  int ret = erlpack_buffer_write(b, (const char *)buf, 5);
  if (ret < 0)
    return ret;

  erlpack_append(b, bytes, size);
}

static inline int erlpack_append_string(erlpack_buffer *b, const char *bytes, size_t size) {
  unsigned char buf[3];
  buf[0] = STRING_EXT;

  _erlpack_store16(buf + 1, size);

  int ret = erlpack_buffer_write(b, (const char *)buf, 3);
  if (ret < 0)
    return ret;

  erlpack_append(b, bytes, size);
}

static inline int erlpack_append_tuple_header(erlpack_buffer *b, size_t size) {
  if (size < 256) {
    unsigned char buf[2];
    buf[0] = SMALL_TUPLE_EXT;
    buf[1] = (unsigned char)size;
    erlpack_append(b, buf, 2);
  } else {
    unsigned char buf[5];
    buf[0] = LARGE_TUPLE_EXT;
    _erlpack_store32(buf + 1, size);
    erlpack_append(b, buf, 5);
  }
}

static inline int erlpack_append_nil_ext(erlpack_buffer *b) {
  static unsigned char buf[1] = {NIL_EXT};
  erlpack_append(b, buf, 1);
}

static inline int erlpack_append_list_header(erlpack_buffer *b, size_t size) {
  unsigned char buf[5];
  buf[0] = LIST_EXT;
  _erlpack_store32(buf + 1, size);
  erlpack_append(b, buf, 5);
}

static inline int erlpack_append_map_header(erlpack_buffer *b, size_t size) {
  unsigned char buf[5];
  buf[0] = MAP_EXT;
  _erlpack_store32(buf + 1, size);
  erlpack_append(b, buf, 5);
}

#ifdef __cplusplus
}

#endif
