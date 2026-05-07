/* clang-format off */
#include "cmp_pty.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

/* Cross-platform mock implementation for tests */

struct cmp_pty {
  int cols;
  int rows;
  char buffer[1024];
  unsigned int buffer_len;
};

/**
 * @brief cmp_pty_create
 *
 * @param out_pty Parameter description.
 * @param command Parameter description.
 * @param cols Parameter description.
 * @param rows Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_create(cmp_pty_t **out_pty, const char *command, int cols,
                   int rows) {
  int rc = CMP_SUCCESS;
  cmp_pty_t *pty = NULL;

  if (out_pty == NULL || command == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pty_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_pty_t), (void **)&pty);
  if (rc != CMP_SUCCESS || pty == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_pty_create: Out of memory\n");
    return rc;
  }

  pty->cols = cols;
  pty->rows = rows;
  pty->buffer_len = 0;

  /* Mock initial output */
#if defined(_MSC_VER)
  if (strncpy_s(pty->buffer, sizeof(pty->buffer), "Simulated PTY start",
                _TRUNCATE) != 0) {
    if (CMP_FREE(pty) != CMP_SUCCESS) {
      LOG_DEBUG("cmp_pty_create: CMP_FREE failed during recovery\n");
    }
    rc = CMP_ERROR_GENERAL;
    LOG_DEBUG("Error in cmp_pty_create: strncpy_s failed\n");
    return rc;
  }
#else
  strncpy(pty->buffer, "Simulated PTY start", 1023);
#endif
  pty->buffer[1023] = '\0';
  pty->buffer_len = (unsigned int)strlen(pty->buffer);

  *out_pty = pty;
  return rc;
}

/**
 * @brief cmp_pty_destroy
 *
 * @param pty Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_destroy(cmp_pty_t *pty) {
  int rc = CMP_SUCCESS;

  if (pty == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pty_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(pty);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pty_destroy: CMP_FREE failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief cmp_pty_resize
 *
 * @param pty Parameter description.
 * @param cols Parameter description.
 * @param rows Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_resize(cmp_pty_t *pty, int cols, int rows) {
  int rc = CMP_SUCCESS;

  if (pty == NULL || cols <= 0 || rows <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pty_resize: Invalid argument\n");
    return rc;
  }

  pty->cols = cols;
  pty->rows = rows;
  return rc;
}

/**
 * @brief cmp_pty_write
 *
 * @param pty Parameter description.
 * @param data Parameter description.
 * @param len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_write(cmp_pty_t *pty, const void *data, unsigned int len) {
  int rc = CMP_SUCCESS;

  if (pty == NULL || data == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pty_write: Invalid argument\n");
    return rc;
  }

  /* Mock echo behavior */
  if (len > 1023) {
    len = 1023;
  }
  memcpy(pty->buffer, data, len);
  pty->buffer_len = len;
  return rc;
}

/**
 * @brief cmp_pty_read
 *
 * @param pty Parameter description.
 * @param out_buffer Parameter description.
 * @param buffer_size Parameter description.
 * @param out_read Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_read(cmp_pty_t *pty, void *out_buffer, unsigned int buffer_size,
                 unsigned int *out_read) {
  int rc = CMP_SUCCESS;
  unsigned int to_copy;

  if (pty == NULL || out_buffer == NULL || out_read == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pty_read: Invalid argument\n");
    return rc;
  }

  if (pty->buffer_len == 0) {
    *out_read = 0;
    return rc;
  }

  to_copy = pty->buffer_len > buffer_size ? buffer_size : pty->buffer_len;
  memcpy(out_buffer, pty->buffer, to_copy);
  *out_read = to_copy;

  /* Consume mock buffer */
  if (to_copy < pty->buffer_len) {
    memmove(pty->buffer, pty->buffer + to_copy, pty->buffer_len - to_copy);
    pty->buffer_len -= to_copy;
  } else {
    pty->buffer_len = 0;
  }
  return rc;
}
