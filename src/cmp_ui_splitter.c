/* clang-format off */
#include "cmp_ui_splitter.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_splitter {
  cmp_ui_node_t *node_root;
  int is_vertical;
  float position;
};

int cmp_ui_splitter_create(cmp_ui_splitter_t **out_splitter, int is_vertical,
                           uint32_t color) {
  cmp_ui_splitter_t *splitter;
  int err;

  if (!out_splitter) {
    return CMP_ERROR_INVALID_ARG;
  }

  splitter = (cmp_ui_splitter_t *)malloc(sizeof(cmp_ui_splitter_t));
  if (!splitter) {
    return CMP_ERROR_OOM;
  }

  splitter->is_vertical = is_vertical;
  splitter->position = 0.5f;

  err = cmp_ui_box_create(&splitter->node_root);
  if (err != 0) {
    free(splitter);
    return err;
  }

  splitter->node_root->bg_color = color;

  *out_splitter = splitter;
  return 0;
}

int cmp_ui_splitter_destroy(cmp_ui_splitter_t *splitter) {
  if (!splitter) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(splitter);
  return 0;
}

int cmp_ui_splitter_get_node(cmp_ui_splitter_t *splitter,
                             cmp_ui_node_t **out_node) {
  if (!splitter || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = splitter->node_root;
  return 0;
}

int cmp_ui_splitter_set_position(cmp_ui_splitter_t *splitter, float position) {
  if (!splitter) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (position < 0.0f) {
    position = 0.0f;
  } else if (position > 1.0f) {
    position = 1.0f;
  }

  splitter->position = position;
  return 0;
}
