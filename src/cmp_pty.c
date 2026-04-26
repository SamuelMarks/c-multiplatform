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
  cmp_pty_t *pty;

  if (!out_pty || !command) {
    return -1;
  }

  rc = CMP_MALLOC(sizeof(cmp_pty_t), (void **)&(pty));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  pty->cols = cols;
  pty->rows = rows;
  pty->buffer_len = 0;

  /* Mock initial output */
  strncpy(pty->buffer, "Simulated PTY start", 1023);
  pty->buffer[1023] = '\0';
  pty->buffer_len = (unsigned int)strlen(pty->buffer);

  *out_pty = pty;
  return 0;
}

/**
 * @brief cmp_pty_destroy
 *
 * @param pty Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pty_destroy(cmp_pty_t *pty) {
  int rc = CMP_SUCCESS;
  if (!pty) {
    return -1;
  }
  rc = CMP_FREE(pty);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
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
  if (!pty || cols <= 0 || rows <= 0) {
    return -1;
  }
  pty->cols = cols;
  pty->rows = rows;
  return 0;
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
  if (!pty || !data) {
    return -1;
  }
  /* Mock echo behavior */
  if (len > 1023) {
    len = 1023;
  }
  memcpy(pty->buffer, data, len);
  pty->buffer_len = len;
  return 0;
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
  unsigned int to_copy;

  if (!pty || !out_buffer || !out_read) {
    return -1;
  }

  if (pty->buffer_len == 0) {
    *out_read = 0;
    return 0;
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

  return 0;
}
