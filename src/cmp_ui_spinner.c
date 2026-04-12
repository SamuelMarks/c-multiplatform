/* clang-format off */
#include "cmp_ui_spinner.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_spinner {
  cmp_ui_node_t *node_root;
  float rotation_angle;
  float size;
};

int cmp_ui_spinner_create(cmp_ui_spinner_t **out_spinner, float size,
                          uint32_t color) {
  cmp_ui_spinner_t *spinner;
  int err;

  if (!out_spinner) {
    return CMP_ERROR_INVALID_ARG;
  }

  spinner = (cmp_ui_spinner_t *)malloc(sizeof(cmp_ui_spinner_t));
  if (!spinner) {
    return CMP_ERROR_OOM;
  }

  spinner->rotation_angle = 0.0f;
  spinner->size = size;

  err = cmp_ui_box_create(&spinner->node_root);
  if (err != 0) {
    free(spinner);
    return err;
  }

  spinner->node_root->bg_color = color;

  *out_spinner = spinner;
  return 0;
}

int cmp_ui_spinner_destroy(cmp_ui_spinner_t *spinner) {
  if (!spinner) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(spinner);
  return 0;
}

int cmp_ui_spinner_get_node(cmp_ui_spinner_t *spinner,
                            cmp_ui_node_t **out_node) {
  if (!spinner || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = spinner->node_root;
  return 0;
}

int cmp_ui_spinner_update(cmp_ui_spinner_t *spinner, float dt_ms) {
  if (!spinner) {
    return CMP_ERROR_INVALID_ARG;
  }

  spinner->rotation_angle += (dt_ms / 1000.0f) * 360.0f;
  if (spinner->rotation_angle >= 360.0f) {
    spinner->rotation_angle -= 360.0f;
  }

  return 0;
}
