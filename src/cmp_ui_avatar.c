/* clang-format off */
#include "cmp_ui_avatar.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_avatar {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *initials;
};

int cmp_ui_avatar_create(cmp_ui_avatar_t **out_avatar, const char *initials,
                         uint32_t bg_color, uint32_t text_color) {
  cmp_ui_avatar_t *avatar;
  int err;
  size_t len;

  if (!out_avatar) {
    return CMP_ERROR_INVALID_ARG;
  }

  avatar = (cmp_ui_avatar_t *)malloc(sizeof(cmp_ui_avatar_t));
  if (!avatar) {
    return CMP_ERROR_OOM;
  }

  avatar->initials = NULL;
  if (initials) {
    len = strlen(initials);
    avatar->initials = (char *)malloc(len + 1);
    if (avatar->initials) {
      memcpy(avatar->initials, initials, len + 1);
    }
  }

  err = cmp_ui_box_create(&avatar->node_root);
  if (err != 0) {
    free(avatar->initials);
    free(avatar);
    return err;
  }

  avatar->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&avatar->node_text,
                           avatar->initials ? avatar->initials : "", -1);
  if (err != 0) {
    free(avatar->initials);
    free(avatar);
    return err;
  }

  avatar->node_text->text_color = text_color;

  cmp_ui_node_add_child(avatar->node_root, avatar->node_text);

  *out_avatar = avatar;
  return 0;
}

int cmp_ui_avatar_destroy(cmp_ui_avatar_t *avatar) {
  if (!avatar) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(avatar->initials);
  free(avatar);
  return 0;
}

int cmp_ui_avatar_get_node(cmp_ui_avatar_t *avatar, cmp_ui_node_t **out_node) {
  if (!avatar || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = avatar->node_root;
  return 0;
}

int cmp_ui_avatar_set_initials(cmp_ui_avatar_t *avatar, const char *initials) {
  size_t len;

  if (!avatar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (avatar->initials) {
    free(avatar->initials);
    avatar->initials = NULL;
  }

  if (initials) {
    len = strlen(initials);
    avatar->initials = (char *)malloc(len + 1);
    if (!avatar->initials) {
      return CMP_ERROR_OOM;
    }
    memcpy(avatar->initials, initials, len + 1);
  }

  return 0;
}
