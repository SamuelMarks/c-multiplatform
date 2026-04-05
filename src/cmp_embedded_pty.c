/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>

/* This is a portable stub. 
   A real implementation utilizes ConPTY on Windows >= 10, or forkpty/openpty on POSIX. */

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
/* clang-format on */

struct cmp_embedded_pty {
  int is_running;
  char mock_buffer[1024];
  size_t mock_len;
  size_t mock_pos;
};

int cmp_embedded_pty_create(cmp_embedded_pty_t **out_pty) {
  cmp_embedded_pty_t *pty;
  if (!out_pty)
    return CMP_ERROR_INVALID_ARG;

  pty = (cmp_embedded_pty_t *)malloc(sizeof(cmp_embedded_pty_t));
  if (!pty)
    return CMP_ERROR_OOM;

  pty->is_running = 0;
  pty->mock_len = 0;
  pty->mock_pos = 0;

  *out_pty = pty;
  return CMP_SUCCESS;
}

int cmp_embedded_pty_destroy(cmp_embedded_pty_t *pty) {
  if (!pty)
    return CMP_ERROR_INVALID_ARG;
  free(pty);
  return CMP_SUCCESS;
}

int cmp_embedded_pty_spawn(cmp_embedded_pty_t *pty, const char *command) {
  const char *welcome = "C:\\> ";

  if (!pty || !command)
    return CMP_ERROR_INVALID_ARG;

  pty->is_running = 1;
  strncpy(pty->mock_buffer, welcome, sizeof(pty->mock_buffer) - 1);
  pty->mock_buffer[sizeof(pty->mock_buffer) - 1] = '\0';
  pty->mock_len = strlen(pty->mock_buffer);
  pty->mock_pos = 0;

  return CMP_SUCCESS;
}

int cmp_embedded_pty_write(cmp_embedded_pty_t *pty, const char *input,
                           size_t length) {
  if (!pty || (!input && length > 0))
    return CMP_ERROR_INVALID_ARG;
  if (!pty->is_running)
    return CMP_ERROR_INVALID_STATE;

  /* Mock echoing the input back */
  if (pty->mock_len + length < sizeof(pty->mock_buffer)) {
    memcpy(&pty->mock_buffer[pty->mock_len], input, length);
    pty->mock_len += length;
    pty->mock_buffer[pty->mock_len] = '\0';
  }

  return CMP_SUCCESS;
}

int cmp_embedded_pty_read(cmp_embedded_pty_t *pty, char *out_buffer,
                          size_t max_len, size_t *out_read) {
  size_t available;
  size_t to_read;

  if (!pty || !out_buffer || !out_read || max_len == 0)
    return CMP_ERROR_INVALID_ARG;
  if (!pty->is_running) {
    *out_read = 0;
    return CMP_SUCCESS;
  }

  available = pty->mock_len - pty->mock_pos;
  if (available == 0) {
    *out_read = 0;
    return CMP_SUCCESS;
  }

  to_read = (available < max_len) ? available : max_len;
  memcpy(out_buffer, &pty->mock_buffer[pty->mock_pos], to_read);
  pty->mock_pos += to_read;

  *out_read = to_read;
  return CMP_SUCCESS;
}
