/* clang-format off */
#include "cmp_ui_avatar.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Avatar widget.
 */
struct cmp_ui_avatar {
  /** @brief The root box node of the avatar */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the avatar initials */
  cmp_ui_node_t *node_text;
  /** @brief The raw string of the initials */
  char *initials;
};

/**
 * @brief cmp_ui_avatar_create
 *
 * @param out_avatar Pointer to output the newly created avatar.
 * @param initials The initials text for the avatar.
 * @param bg_color The background color.
 * @param text_color The text color.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_create(cmp_ui_avatar_t **out_avatar, const char *initials,
                         uint32_t bg_color, uint32_t text_color) {
  cmp_ui_avatar_t *avatar;
  int rc;
  size_t len;

  if (!out_avatar) {
    LOG_DEBUG("cmp_ui_avatar_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_avatar_t), (void **)&avatar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  avatar->initials = NULL;
  if (initials) {
    len = strlen(initials);
    rc = CMP_MALLOC(len + 1, (void **)&avatar->initials);
    if (rc == CMP_SUCCESS) {
      memcpy(avatar->initials, initials, len + 1);
    } else {
      LOG_DEBUG("cmp_ui_avatar_create: OOM initials\n");
      rc = CMP_FREE(avatar);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_avatar_create: CMP_FREE failed\n");
      }
      return CMP_ERROR_OOM;
    }
  }

  rc = cmp_ui_box_create(&avatar->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_box_create failed\n");
    if (avatar->initials) {
      rc = CMP_FREE(avatar->initials);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_avatar_create: CMP_FREE initials failed\n");
      }
    }
    rc = CMP_FREE(avatar);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_create: CMP_FREE avatar failed\n");
    }
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  avatar->node_root->bg_color = bg_color;

  rc = cmp_ui_text_create(&avatar->node_text,
                          avatar->initials ? avatar->initials : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(avatar->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_node_destroy failed\n");
    }
    if (avatar->initials) {
      rc = CMP_FREE(avatar->initials);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_avatar_create: CMP_FREE initials failed\n");
      }
    }
    rc = CMP_FREE(avatar);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_create: CMP_FREE avatar failed\n");
    }
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  avatar->node_text->text_color = text_color;

  rc = cmp_ui_node_add_child(avatar->node_root, avatar->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_node_add_child failed\n");
  }

  *out_avatar = avatar;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_avatar_destroy
 *
 * @param avatar The avatar component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_destroy(cmp_ui_avatar_t *avatar) {
  int rc;
  if (!avatar) {
    LOG_DEBUG("cmp_ui_avatar_destroy: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (avatar->initials) {
    rc = CMP_FREE(avatar->initials);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_destroy: CMP_FREE initials failed\n");
    }
  }
  if (avatar->node_root) {
    rc = cmp_ui_node_destroy(avatar->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(avatar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_destroy: CMP_FREE failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_avatar_get_node
 *
 * @param avatar The avatar component.
 * @param out_node Pointer to receive the node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_get_node(cmp_ui_avatar_t *avatar, cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  if (!avatar || !out_node) {
    LOG_DEBUG("cmp_ui_avatar_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = avatar->node_root;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_avatar_set_initials
 *
 * @param avatar The avatar component.
 * @param initials The initials to set.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_set_initials(cmp_ui_avatar_t *avatar, const char *initials) {
  size_t len;
  int rc;

  if (!avatar) {
    LOG_DEBUG("cmp_ui_avatar_set_initials: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (avatar->initials) {
    rc = CMP_FREE(avatar->initials);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_set_initials: CMP_FREE initials failed\n");
    }
    avatar->initials = NULL;
  }

  if (initials) {
    len = strlen(initials);
    rc = CMP_MALLOC(len + 1, (void **)&avatar->initials);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_set_initials: OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(avatar->initials, initials, len + 1);
  }

  return CMP_SUCCESS;
}