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

  struct cmp_attachment_pill attachments[MAX_ATTACHMENTS];
  size_t attachment_count;
};

/**
 * @brief cmp_input_layout_create
 *
 * @param out_layout Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_layout_create(cmp_input_layout_t **out_layout) {
  int rc = CMP_SUCCESS;
  struct cmp_input_layout *layout = NULL;

  if (!out_layout) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_input_layout), (void **)&layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_input_layout_create: Out of memory\n");
    return rc;
  }

  layout->line_height = 20.0f;
  layout->max_lines = 10;
  layout->attachment_count = 0;

  *out_layout = (cmp_input_layout_t *)layout;
  return rc;
}

/**
 * @brief cmp_input_layout_destroy
 *
 * @param layout_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_layout_destroy(cmp_input_layout_t *layout_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_input_layout *layout = (struct cmp_input_layout *)layout_opaque;

  if (!layout) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_layout_destroy: Invalid argument\n");
    return rc;
  }
  rc = CMP_FREE(layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_input_layout_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief cmp_input_layout_update_text
 *
 * @param layout_opaque Parameter description.
 * @param new_text Parameter description.
 * @param out_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_layout_update_text(cmp_input_layout_t *layout_opaque,
                                 const char *new_text, float *out_height) {
  int rc = CMP_SUCCESS;
  struct cmp_input_layout *layout = (struct cmp_input_layout *)layout_opaque;
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

/**
 * @brief cmp_input_layout_add_attachment
 *
 * @param layout_opaque Parameter description.
 * @param filename Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_layout_add_attachment(cmp_input_layout_t *layout_opaque,
                                    const char *filename) {
  int rc = CMP_SUCCESS;
  struct cmp_input_layout *layout = (struct cmp_input_layout *)layout_opaque;

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

#if defined(_MSC_VER)
  strcpy_s(layout->attachments[layout->attachment_count].filename, 256,
           filename);
#else
  strncpy(layout->attachments[layout->attachment_count].filename, filename,
          255);
  layout->attachments[layout->attachment_count].filename[255] = '\0';
#endif
  layout->attachment_count++;

  return rc;
}

/**
 * @brief cmp_input_layout_get_attachment_count
 *
 * @param layout_opaque Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_layout_get_attachment_count(
    const cmp_input_layout_t *layout_opaque, size_t *out_count) {
  int rc = CMP_SUCCESS;
  const struct cmp_input_layout *layout =
      (const struct cmp_input_layout *)layout_opaque;

  if (!layout || !out_count) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_input_layout_get_attachment_count: Invalid argument\n");
    return rc;
  }

  *out_count = layout->attachment_count;
  return rc;
}
