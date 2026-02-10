#include "m3/m3_utf8.h"

#ifdef M3_TESTING
static m3_usize m3_utf8_test_limit_value = 0;
static M3Bool g_m3_utf8_relaxed_checks = M3_FALSE;
#endif

static m3_usize m3_usize_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static m3_usize m3_utf8_cstr_limit(void) {
#ifdef M3_TESTING
  if (m3_utf8_test_limit_value != 0) {
    return m3_utf8_test_limit_value;
  }
#endif
  return m3_usize_max_value();
}

static int m3_utf8_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize max_len;
  m3_usize length;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_len = m3_utf8_cstr_limit();
  length = 0;

  while (cstr[length] != '\0') {
    if (length == max_len) {
      return M3_ERR_OVERFLOW;
    }
    length += 1;
  }

  *out_len = length;
  return M3_OK;
}

static int m3_utf8_decode(const m3_u8 *data, m3_usize length,
                          m3_u32 *out_codepoint, m3_usize *out_advance) {
  m3_u8 b0;
  m3_u8 b1;
  m3_u8 b2;
  m3_u8 b3;
  m3_u32 codepoint;

  if (data == NULL || out_codepoint == NULL || out_advance == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (length == 0) {
    return M3_ERR_NOT_FOUND;
  }

  b0 = data[0];
  if (b0 < 0x80u) {
    *out_codepoint = b0;
    *out_advance = 1;
    return M3_OK;
  }

  if (b0 < 0xC2u) {
    return M3_ERR_CORRUPT;
  }

  if (b0 <= 0xDFu) {
    if (length < 2) {
      return M3_ERR_CORRUPT;
    }
    b1 = data[1];
    if (b1 < 0x80u || b1 > 0xBFu) {
      return M3_ERR_CORRUPT;
    }
    codepoint = (m3_u32)((b0 & 0x1Fu) << 6);
    codepoint |= (m3_u32)(b1 & 0x3Fu);
    *out_codepoint = codepoint;
    *out_advance = 2;
    return M3_OK;
  }

  if (b0 <= 0xEFu) {
    if (length < 3) {
      return M3_ERR_CORRUPT;
    }
    b1 = data[1];
    b2 = data[2];
    if (b1 < 0x80u || b1 > 0xBFu || b2 < 0x80u || b2 > 0xBFu) {
      return M3_ERR_CORRUPT;
    }
#ifdef M3_TESTING
    if (!g_m3_utf8_relaxed_checks) {
      if (b0 == 0xE0u && b1 < 0xA0u) {
        return M3_ERR_CORRUPT;
      }
      if (b0 == 0xEDu && b1 > 0x9Fu) {
        return M3_ERR_CORRUPT;
      }
    }
#else
    if (b0 == 0xE0u && b1 < 0xA0u) {
      return M3_ERR_CORRUPT;
    }
    if (b0 == 0xEDu && b1 > 0x9Fu) {
      return M3_ERR_CORRUPT;
    }
#endif
    codepoint = (m3_u32)((b0 & 0x0Fu) << 12);
    codepoint |= (m3_u32)((b1 & 0x3Fu) << 6);
    codepoint |= (m3_u32)(b2 & 0x3Fu);
    if (codepoint >= 0xD800u && codepoint <= 0xDFFFu) {
      return M3_ERR_CORRUPT;
    }
    *out_codepoint = codepoint;
    *out_advance = 3;
    return M3_OK;
  }

  if (b0 <= 0xF4u) {
    if (length < 4) {
      return M3_ERR_CORRUPT;
    }
    b1 = data[1];
    b2 = data[2];
    b3 = data[3];
    if (b1 < 0x80u || b1 > 0xBFu || b2 < 0x80u || b2 > 0xBFu || b3 < 0x80u ||
        b3 > 0xBFu) {
      return M3_ERR_CORRUPT;
    }
#ifdef M3_TESTING
    if (!g_m3_utf8_relaxed_checks) {
      if (b0 == 0xF0u && b1 < 0x90u) {
        return M3_ERR_CORRUPT;
      }
      if (b0 == 0xF4u && b1 > 0x8Fu) {
        return M3_ERR_CORRUPT;
      }
    }
#else
    if (b0 == 0xF0u && b1 < 0x90u) {
      return M3_ERR_CORRUPT;
    }
    if (b0 == 0xF4u && b1 > 0x8Fu) {
      return M3_ERR_CORRUPT;
    }
#endif
    codepoint = (m3_u32)((b0 & 0x07u) << 18);
    codepoint |= (m3_u32)((b1 & 0x3Fu) << 12);
    codepoint |= (m3_u32)((b2 & 0x3Fu) << 6);
    codepoint |= (m3_u32)(b3 & 0x3Fu);
    if (codepoint > 0x10FFFFu) {
      return M3_ERR_CORRUPT;
    }
    *out_codepoint = codepoint;
    *out_advance = 4;
    return M3_OK;
  }

  return M3_ERR_CORRUPT;
}

int M3_CALL m3_utf8_validate(const char *data, m3_usize length,
                             M3Bool *out_valid) {
  const m3_u8 *bytes;
  m3_usize offset;
  m3_usize advance;
  m3_u32 codepoint;
  int rc;

  if (data == NULL || out_valid == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  bytes = (const m3_u8 *)data;
  offset = 0;
  *out_valid = M3_TRUE;

  while (offset < length) {
    rc = m3_utf8_decode(bytes + offset, length - offset, &codepoint, &advance);
    if (rc != M3_OK) {
      *out_valid = M3_FALSE;
      return M3_OK;
    }
    offset += advance;
  }

  return M3_OK;
}

int M3_CALL m3_utf8_validate_cstr(const char *cstr, M3Bool *out_valid) {
  m3_usize length;
  int rc;

  if (out_valid == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_valid = M3_FALSE;

  rc = m3_utf8_cstrlen(cstr, &length);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_utf8_validate(cstr, length, out_valid);
}

int M3_CALL m3_utf8_iter_init(M3Utf8Iter *iter, const char *data,
                              m3_usize length) {
  if (iter == NULL || data == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  iter->data = (const m3_u8 *)data;
  iter->length = length;
  iter->offset = 0;
  return M3_OK;
}

int M3_CALL m3_utf8_iter_init_cstr(M3Utf8Iter *iter, const char *cstr) {
  m3_usize length;
  int rc;

  if (iter == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_utf8_cstrlen(cstr, &length);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_utf8_iter_init(iter, cstr, length);
}

int M3_CALL m3_utf8_iter_next(M3Utf8Iter *iter, m3_u32 *out_codepoint) {
  m3_usize advance;
  int rc;

  if (iter == NULL || out_codepoint == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (iter->offset >= iter->length) {
    return M3_ERR_NOT_FOUND;
  }

  rc = m3_utf8_decode(iter->data + iter->offset, iter->length - iter->offset,
                      out_codepoint, &advance);
  if (rc != M3_OK) {
    return rc;
  }

  iter->offset += advance;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_utf8_test_set_cstr_limit(m3_usize max_len) {
  if (max_len == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  m3_utf8_test_limit_value = max_len;
  return M3_OK;
}

int M3_CALL m3_utf8_test_decode(const m3_u8 *data, m3_usize length,
                                m3_u32 *out_codepoint, m3_usize *out_advance) {
  return m3_utf8_decode(data, length, out_codepoint, out_advance);
}

int M3_CALL m3_utf8_test_set_relaxed_checks(M3Bool enable) {
  g_m3_utf8_relaxed_checks = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
