/* clang-format off */
#include "cmp.h"
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
  cmp_input_layout_t *layout;

  if (!out_layout)
    return CMP_ERROR_INVALID_ARG;

  layout = (cmp_input_layout_t *)malloc(sizeof(cmp_input_layout_t));
  if (!layout)
    return CMP_ERROR_OOM;

  layout->line_height = 20.0f;
  layout->max_lines = 10;
  layout->attachment_count = 0;

  *out_layout = layout;
  return CMP_SUCCESS;
}

int cmp_input_layout_destroy(cmp_input_layout_t *layout) {
  if (!layout)
    return CMP_ERROR_INVALID_ARG;
  free(layout);
  return CMP_SUCCESS;
}

int cmp_input_layout_update_text(cmp_input_layout_t *layout,
                                 const char *new_text, float *out_height) {
  int num_lines = 1;
  const char *p;

  if (!layout || !new_text || !out_height) {
    return CMP_ERROR_INVALID_ARG;
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

  return CMP_SUCCESS;
}

int cmp_input_layout_add_attachment(cmp_input_layout_t *layout,
                                    const char *filename) {
  if (!layout || !filename) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (layout->attachment_count >= MAX_ATTACHMENTS) {
    return CMP_ERROR_BOUNDS;
  }

  strncpy(layout->attachments[layout->attachment_count].filename, filename,
          255);
  layout->attachments[layout->attachment_count].filename[255] = '\0';
  layout->attachment_count++;

  return CMP_SUCCESS;
}

int cmp_input_layout_get_attachment_count(const cmp_input_layout_t *layout,
                                          size_t *out_count) {
  if (!layout || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_count = layout->attachment_count;
  return CMP_SUCCESS;
}
