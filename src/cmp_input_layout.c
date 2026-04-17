/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define MAX_ATTACHMENTS 16

struct cmp_attachment_pill {
  char filename[256];
};

struct cmp_input_layout {
  float line_height;
  int max_lines;

  cmp_attachment_pill_t attachments[MAX_ATTACHMENTS];
  size_t attachment_count;
};

int cmp_input_layout_create(cmp_input_layout_t **out_layout) {
  int rc = CMP_SUCCESS;
  cmp_input_layout_t *layout = NULL;

  if (!out_layout) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_input_layout_t), (void **)&layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_input_layout_create: Out of memory\n");
    return rc;
  }

  layout->line_height = 20.0f;
  layout->max_lines = 10;
  layout->attachment_count = 0;

  *out_layout = layout;
  return rc;
}

int cmp_input_layout_destroy(cmp_input_layout_t *layout) {
  int rc = CMP_SUCCESS;

  if (!layout) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_destroy: Invalid argument\n");
    return rc;
  }
  free(layout);
  return rc;
}

int cmp_input_layout_update_text(cmp_input_layout_t *layout,
                                 const char *new_text, float *out_height) {
  int rc = CMP_SUCCESS;
  int num_lines = 1;
  const char *p;

  if (!layout || !new_text || !out_height) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_update_text: Invalid argument\n");
    return rc;
  }

  /* Count newlines */
  for (p = new_text; *p; p++) {
    if (*p == '\n') {
      num_lines++;
    }
  }

  if (num_lines > layout->max_lines) {
    num_lines = layout->max_lines;
  }

  /* Padding for UI */
  *out_height = (num_lines * layout->line_height) + 16.0f;

  /* Additional height for pills */
  if (layout->attachment_count > 0) {
    *out_height += 30.0f;
  }

  return rc;
}

int cmp_input_layout_add_attachment(cmp_input_layout_t *layout,
                                    const char *filename) {
  int rc = CMP_SUCCESS;

  if (!layout || !filename) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_add_attachment: Invalid argument\n");
    return rc;
  }

  if (layout->attachment_count >= MAX_ATTACHMENTS) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG(
        "Error in cmp_input_layout_add_attachment: Max attachments reached\n");
    return rc;
  }

  strncpy(layout->attachments[layout->attachment_count].filename, filename,
          255);
  layout->attachments[layout->attachment_count].filename[255] = '\0';
  layout->attachment_count++;

  return rc;
}

int cmp_input_layout_get_attachment_count(const cmp_input_layout_t *layout,
                                          size_t *out_count) {
  int rc = CMP_SUCCESS;

  if (!layout || !out_count) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_input_layout_get_attachment_count: Invalid argument\n");
    return rc;
  }

  *out_count = layout->attachment_count;
  return rc;
}
