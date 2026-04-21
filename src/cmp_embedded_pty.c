/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>

/* This is a portable mockup implementation.
   A full version utilizing ConPTY on Windows >= 10, or forkpty/openpty on POSIX,
   will be provided in a later phase. */

/* clang-format on */

struct cmp_embedded_pty {
  int is_running;
  char mock_buffer[1024];
  size_t mock_len;
  size_t mock_pos;
};

int cmp_embedded_pty_create(cmp_embedded_pty_t **out_pty) {
  int rc = CMP_SUCCESS;
  cmp_embedded_pty_t *pty = NULL;

  if (!out_pty) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_embedded_pty_create: Invalid argument (out_pty=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_embedded_pty), (void **)&pty);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_embedded_pty_create: Out of memory\n");
    return rc;
  }

  pty->is_running = 0;
  pty->mock_len = 0;
  pty->mock_pos = 0;

  *out_pty = pty;
  return rc;
}

int cmp_embedded_pty_destroy(cmp_embedded_pty_t *pty) {
  int rc = CMP_SUCCESS;

  if (!pty) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_embedded_pty_destroy: Invalid argument (pty=NULL)\n");
    return rc;
  }

  CMP_FREE(pty);
  return rc;
}

int cmp_embedded_pty_spawn(cmp_embedded_pty_t *pty, const char *command) {
  int rc = CMP_SUCCESS;
  const char *welcome = "C:\\> ";

  if (!pty || !command) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_embedded_pty_spawn: Invalid argument\n");
    return rc;
  }

  pty->is_running = 1;
  strncpy(pty->mock_buffer, welcome, sizeof(pty->mock_buffer) - 1);
  pty->mock_buffer[sizeof(pty->mock_buffer) - 1] = '\0';
  pty->mock_len = strlen(pty->mock_buffer);
  pty->mock_pos = 0;

  return rc;
}

int cmp_embedded_pty_write(cmp_embedded_pty_t *pty, const char *input,
                           size_t length) {
  int rc = CMP_SUCCESS;

  if (!pty || (!input && length > 0)) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_embedded_pty_write: Invalid argument\n");
    return rc;
  }

  if (!pty->is_running) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_embedded_pty_write: PTY is not running\n");
    return rc;
  }

  /* Mock echoing the input back */
  if (pty->mock_len + length < sizeof(pty->mock_buffer)) {
    memcpy(&pty->mock_buffer[pty->mock_len], input, length);
    pty->mock_len += length;
    pty->mock_buffer[pty->mock_len] = '\0';
  }

  return rc;
}

int cmp_embedded_pty_read(cmp_embedded_pty_t *pty, char *out_buffer,
                          size_t max_len, size_t *out_read) {
  int rc = CMP_SUCCESS;
  size_t available;
  size_t to_read;

  if (!pty || !out_buffer || !out_read || max_len == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_embedded_pty_read: Invalid argument\n");
    return rc;
  }

  if (!pty->is_running) {
    *out_read = 0;
    return rc;
  }

  available = pty->mock_len - pty->mock_pos;
  if (available == 0) {
    *out_read = 0;
    return rc;
  }

  to_read = (available < max_len) ? available : max_len;
  memcpy(out_buffer, &pty->mock_buffer[pty->mock_pos], to_read);
  pty->mock_pos += to_read;

  *out_read = to_read;
  return rc;
}
