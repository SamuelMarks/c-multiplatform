#include "cmpc/cmp_utf8.h"

#ifdef CMP_TESTING
static cmp_usize cmp_utf8_test_limit_value = 0;
static CMPBool g_cmp_utf8_relaxed_checks = CMP_FALSE;
#endif

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static cmp_usize cmp_utf8_cstr_limit(void) {
#ifdef CMP_TESTING
  if (cmp_utf8_test_limit_value != 0) {
    return cmp_utf8_test_limit_value;
  }
#endif
  return cmp_usize_max_value();
}

static int cmp_utf8_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize max_len;
  cmp_usize length;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_len = cmp_utf8_cstr_limit();
  length = 0;

  while (cstr[length] != '\0') {
    if (length == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    length += 1;
  }

  *out_len = length;
  return CMP_OK;
}

static int cmp_utf8_decode(const cmp_u8 *data, cmp_usize length,
                           cmp_u32 *out_codepoint, cmp_usize *out_advance) {
  cmp_u8 b0;
  cmp_u8 b1;
  cmp_u8 b2;
  cmp_u8 b3;
  cmp_u32 codepoint;

  if (data == NULL || out_codepoint == NULL || out_advance == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (length == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  b0 = data[0];
  if (b0 < 0x80u) {
    *out_codepoint = b0;
    *out_advance = 1;
    return CMP_OK;
  }

  if (b0 < 0xC2u) {
    return CMP_ERR_CORRUPT;
  }

  if (b0 <= 0xDFu) {
    if (length < 2) {
      return CMP_ERR_CORRUPT;
    }
    b1 = data[1];
    if (b1 < 0x80u || b1 > 0xBFu) {
      return CMP_ERR_CORRUPT;
    }
    codepoint = (cmp_u32)((b0 & 0x1Fu) << 6);
    codepoint |= (cmp_u32)(b1 & 0x3Fu);
    *out_codepoint = codepoint;
    *out_advance = 2;
    return CMP_OK;
  }

  if (b0 <= 0xEFu) {
    if (length < 3) {
      return CMP_ERR_CORRUPT;
    }
    b1 = data[1];
    b2 = data[2];
    if (b1 < 0x80u || b1 > 0xBFu || b2 < 0x80u || b2 > 0xBFu) {
      return CMP_ERR_CORRUPT;
    }
#ifdef CMP_TESTING
    if (!g_cmp_utf8_relaxed_checks) {
      if (b0 == 0xE0u && b1 < 0xA0u) {
        return CMP_ERR_CORRUPT;
      }
      if (b0 == 0xEDu && b1 > 0x9Fu) {
        return CMP_ERR_CORRUPT;
      }
    }
#else
    if (b0 == 0xE0u && b1 < 0xA0u) {
      return CMP_ERR_CORRUPT;
    }
    if (b0 == 0xEDu && b1 > 0x9Fu) {
      return CMP_ERR_CORRUPT;
    }
#endif
    codepoint = (cmp_u32)((b0 & 0x0Fu) << 12);
    codepoint |= (cmp_u32)((b1 & 0x3Fu) << 6);
    codepoint |= (cmp_u32)(b2 & 0x3Fu);
    if (codepoint >= 0xD800u && codepoint <= 0xDFFFu) {
      return CMP_ERR_CORRUPT;
    }
    *out_codepoint = codepoint;
    *out_advance = 3;
    return CMP_OK;
  }

  if (b0 <= 0xF4u) {
    if (length < 4) {
      return CMP_ERR_CORRUPT;
    }
    b1 = data[1];
    b2 = data[2];
    b3 = data[3];
    if (b1 < 0x80u || b1 > 0xBFu || b2 < 0x80u || b2 > 0xBFu || b3 < 0x80u ||
        b3 > 0xBFu) {
      return CMP_ERR_CORRUPT;
    }
#ifdef CMP_TESTING
    if (!g_cmp_utf8_relaxed_checks) {
      if (b0 == 0xF0u && b1 < 0x90u) {
        return CMP_ERR_CORRUPT;
      }
      if (b0 == 0xF4u && b1 > 0x8Fu) {
        return CMP_ERR_CORRUPT;
      }
    }
#else
    if (b0 == 0xF0u && b1 < 0x90u) {
      return CMP_ERR_CORRUPT;
    }
    if (b0 == 0xF4u && b1 > 0x8Fu) {
      return CMP_ERR_CORRUPT;
    }
#endif
    codepoint = (cmp_u32)((b0 & 0x07u) << 18);
    codepoint |= (cmp_u32)((b1 & 0x3Fu) << 12);
    codepoint |= (cmp_u32)((b2 & 0x3Fu) << 6);
    codepoint |= (cmp_u32)(b3 & 0x3Fu);
    if (codepoint > 0x10FFFFu) {
      return CMP_ERR_CORRUPT;
    }
    *out_codepoint = codepoint;
    *out_advance = 4;
    return CMP_OK;
  }

  return CMP_ERR_CORRUPT;
}

int CMP_CALL cmp_utf8_validate(const char *data, cmp_usize length,
                               CMPBool *out_valid) {
  const cmp_u8 *bytes;
  cmp_usize offset;
  cmp_usize advance;
  cmp_u32 codepoint;
  int rc;

  if (data == NULL || out_valid == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bytes = (const cmp_u8 *)data;
  offset = 0;
  *out_valid = CMP_TRUE;

  while (offset < length) {
    rc = cmp_utf8_decode(bytes + offset, length - offset, &codepoint, &advance);
    if (rc != CMP_OK) {
      *out_valid = CMP_FALSE;
      return CMP_OK;
    }
    offset += advance;
  }

  return CMP_OK;
}

int CMP_CALL cmp_utf8_validate_cstr(const char *cstr, CMPBool *out_valid) {
  cmp_usize length;
  int rc;

  if (out_valid == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_valid = CMP_FALSE;

  rc = cmp_utf8_cstrlen(cstr, &length);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_utf8_validate(cstr, length, out_valid);
}

int CMP_CALL cmp_utf8_iter_init(CMPUtf8Iter *iter, const char *data,
                                cmp_usize length) {
  if (iter == NULL || data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  iter->data = (const cmp_u8 *)data;
  iter->length = length;
  iter->offset = 0;
  return CMP_OK;
}

int CMP_CALL cmp_utf8_iter_init_cstr(CMPUtf8Iter *iter, const char *cstr) {
  cmp_usize length;
  int rc;

  if (iter == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_utf8_cstrlen(cstr, &length);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_utf8_iter_init(iter, cstr, length);
}

int CMP_CALL cmp_utf8_iter_next(CMPUtf8Iter *iter, cmp_u32 *out_codepoint) {
  cmp_usize advance;
  int rc;

  if (iter == NULL || out_codepoint == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (iter->offset >= iter->length) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_utf8_decode(iter->data + iter->offset, iter->length - iter->offset,
                       out_codepoint, &advance);
  if (rc != CMP_OK) {
    return rc;
  }

  iter->offset += advance;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_utf8_test_set_cstr_limit(cmp_usize max_len) {
  if (max_len == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  cmp_utf8_test_limit_value = max_len;
  return CMP_OK;
}

int CMP_CALL cmp_utf8_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  return cmp_utf8_cstrlen(cstr, out_len);
}

int CMP_CALL cmp_utf8_test_decode(const cmp_u8 *data, cmp_usize length,
                                  cmp_u32 *out_codepoint,
                                  cmp_usize *out_advance) {
  return cmp_utf8_decode(data, length, out_codepoint, out_advance);
}

int CMP_CALL cmp_utf8_test_set_relaxed_checks(CMPBool enable) {
  g_cmp_utf8_relaxed_checks = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
