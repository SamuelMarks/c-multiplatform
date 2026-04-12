/* clang-format off */
#include "cmp_ui_terminal.h"
#include "cmp_pty.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_terminal {
  cmp_ui_node_t *node_root;
  char *buffer;
  size_t buffer_size;
  cmp_pty_t *pty;
};

int cmp_ui_terminal_create(cmp_ui_terminal_t **out_terminal, uint32_t bg_color,
                           uint32_t fg_color) {
  cmp_ui_terminal_t *terminal;
  int err;

  if (!out_terminal) {
    return CMP_ERROR_INVALID_ARG;
  }

  terminal = (cmp_ui_terminal_t *)malloc(sizeof(cmp_ui_terminal_t));
  if (!terminal) {
    return CMP_ERROR_OOM;
  }

  terminal->buffer = NULL;
  terminal->buffer_size = 0;
  terminal->pty = NULL;

  err = cmp_ui_box_create(&terminal->node_root);
  if (err != 0) {
    free(terminal);
    return err;
  }

  terminal->node_root->bg_color = bg_color;
  (void)fg_color; /* To be used when setting up text nodes */

#if defined(_WIN32)
  if (cmp_pty_create(&terminal->pty, "cmd.exe", 80, 24) != 0) {
#else
  if (cmp_pty_create(&terminal->pty, "bash", 80, 24) != 0) {
#endif
    terminal->pty = NULL;
  }

  *out_terminal = terminal;
  return 0;
}

int cmp_ui_terminal_destroy(cmp_ui_terminal_t *terminal) {
  if (!terminal) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (terminal->pty) {
    cmp_pty_destroy(terminal->pty);
  }
  free(terminal->buffer);
  free(terminal);
  return 0;
}

int cmp_ui_terminal_get_node(cmp_ui_terminal_t *terminal,
                             cmp_ui_node_t **out_node) {
  if (!terminal || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = terminal->node_root;
  return 0;
}

int cmp_ui_terminal_append_output(cmp_ui_terminal_t *terminal,
                                  const char *output) {
  size_t len;
  char *new_buf;

  if (!terminal || !output) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (terminal->pty) {
    cmp_pty_write(terminal->pty, output, (unsigned int)strlen(output));
  }

  len = strlen(output);
  new_buf = (char *)malloc(terminal->buffer_size + len + 1);
  if (!new_buf) {
    return CMP_ERROR_OOM;
  }

  if (terminal->buffer) {
    memcpy(new_buf, terminal->buffer, terminal->buffer_size);
    free(terminal->buffer);
  }

  memcpy(new_buf + terminal->buffer_size, output, len + 1);
  terminal->buffer = new_buf;
  terminal->buffer_size += len;

  return 0;
}
