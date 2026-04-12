/* clang-format off */
#include "cmp_ui_diff.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_diff {
  cmp_ui_node_t *node_root;
  char *old_text;
  char *new_text;
};

int cmp_ui_diff_create(cmp_ui_diff_t **out_diff, const char *old_text,
                       const char *new_text) {
  cmp_ui_diff_t *diff;
  int err;
  size_t len;

  if (!out_diff) {
    return CMP_ERROR_INVALID_ARG;
  }

  diff = (cmp_ui_diff_t *)malloc(sizeof(cmp_ui_diff_t));
  if (!diff) {
    return CMP_ERROR_OOM;
  }

  diff->old_text = NULL;
  if (old_text) {
    len = strlen(old_text);
    diff->old_text = (char *)malloc(len + 1);
    if (diff->old_text) {
      memcpy(diff->old_text, old_text, len + 1);
    }
  }

  diff->new_text = NULL;
  if (new_text) {
    len = strlen(new_text);
    diff->new_text = (char *)malloc(len + 1);
    if (diff->new_text) {
      memcpy(diff->new_text, new_text, len + 1);
    }
  }

  err = cmp_ui_box_create(&diff->node_root);
  if (err != 0) {
    free(diff->old_text);
    free(diff->new_text);
    free(diff);
    return err;
  }

  *out_diff = diff;
  return 0;
}

int cmp_ui_diff_destroy(cmp_ui_diff_t *diff) {
  if (!diff) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(diff->old_text);
  free(diff->new_text);
  free(diff);
  return 0;
}

int cmp_ui_diff_get_node(cmp_ui_diff_t *diff, cmp_ui_node_t **out_node) {
  if (!diff || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = diff->node_root;
  return 0;
}
