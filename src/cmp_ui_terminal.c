/* clang-format off */
#include "cmp_ui_terminal.h"
#include "cmp_pty.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_terminal {
  cmp_ui_node_t *node_root;
  char *buffer;
  size_t buffer_size;
  cmp_pty_t *pty;
};

/**
 * @brief Creates a new terminal component.
 *
 * @param out_terminal Pointer to store the created terminal handle.
 * @param bg_color Background color of the terminal (ARGB).
 * @param fg_color Default foreground text color (ARGB).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_terminal_create(cmp_ui_terminal_t **out_terminal, uint32_t bg_color,
                           uint32_t fg_color) {
  cmp_ui_terminal_t *terminal;
  int err;
  int rc = CMP_SUCCESS;

  if (!out_terminal) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_terminal_t), (void **)&(terminal));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  terminal->buffer = NULL;
  terminal->buffer_size = 0;
  terminal->pty = NULL;

  err = cmp_ui_box_create(&terminal->node_root);
  if (err != 0) {
    CMP_FREE(terminal);
    return err;
  }

  terminal->node_root->bg_color = bg_color;
  (void)fg_color; /* To be used when setting up text nodes */

#if defined(_WIN32)
  rc = cmp_pty_create(&terminal->pty, "cmd.exe", CMP_TERMINAL_DEFAULT_COLS,
                      CMP_TERMINAL_DEFAULT_ROWS);
  if (rc != CMP_SUCCESS) {
#else
  rc = cmp_pty_create(&terminal->pty, "bash", CMP_TERMINAL_DEFAULT_COLS,
                      CMP_TERMINAL_DEFAULT_ROWS);
  if (rc != CMP_SUCCESS) {
#endif
    terminal->pty = NULL;
    /* Optional: If pty creation is strictly required, we should destroy and
       return rc here. But assuming it can fall back to a disconnected terminal:
     */
    /* Let's be consistent and return the error while freeing. */
    cmp_ui_node_destroy(terminal->node_root);
    CMP_FREE(terminal);
    return rc;
  }

  *out_terminal = terminal;
  return CMP_SUCCESS;
}

/**
 * @brief Destroys a terminal component.
 *
 * @param terminal The component to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_terminal_destroy(cmp_ui_terminal_t *terminal) {
  int rc = CMP_SUCCESS;
  int temp_rc;
  if (!terminal) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (terminal->pty) {
    (void)cmp_pty_destroy(terminal->pty);
  }
  if (terminal->node_root) {
    temp_rc = cmp_ui_node_destroy(terminal->node_root);
    if (temp_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_terminal_destroy: cmp_ui_node_destroy failed\n");
      rc = temp_rc;
    }
  }
  if (terminal->buffer) {
    temp_rc = CMP_FREE(terminal->buffer);
    if (temp_rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
      rc = temp_rc;
    }
  }
  temp_rc = CMP_FREE(terminal);
  if (temp_rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    rc = temp_rc;
  }
  return rc;
}

/**
 * @brief Retrieves the underlying UI node.
 *
 * @param terminal The terminal component.
 * @param out_node Pointer to store the UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_terminal_get_node(cmp_ui_terminal_t *terminal,
                             cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!terminal || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = terminal->node_root;

  return rc;
}

/**
 * @brief Appends output text to the terminal.
 *
 * @param terminal The terminal component.
 * @param output The string to append.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_terminal_append_output(cmp_ui_terminal_t *terminal,
                                  const char *output) {
  size_t len;
  char *new_buf;
  int rc = CMP_SUCCESS;

  if (!terminal || !output) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (terminal->pty) {
    (void)cmp_pty_write(terminal->pty, output, (unsigned int)strlen(output));
  }

  len = strlen(output);
  rc = CMP_MALLOC(terminal->buffer_size + len + 1, (void **)&(new_buf));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  if (terminal->buffer) {
    memcpy(new_buf, terminal->buffer, terminal->buffer_size);
    rc = CMP_FREE(terminal->buffer);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }

  memcpy(new_buf + terminal->buffer_size, output, len + 1);
  terminal->buffer = new_buf;
  terminal->buffer_size += len;

  return rc;
}
